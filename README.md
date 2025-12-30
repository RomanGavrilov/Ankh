Below is a “deep-architecture” review focused on lifetime / ownership, explicit synchronization, and multi-frame GPU safety (SAFE-CODE style). I’m going to be blunt about the sharp edges.

Highest-risk issues (fix first)
1) Host-visible writes are missing explicit flush/invalidate guarantees

You map CPU→GPU buffers (FrameUBO + Object SSBO) and memcpy every frame (update_uniform_buffer). But there’s no flush of non-coherent memory. Depending on VMA configuration + memory type chosen, this can become “works on my GPU” UB-in-practice. 

pasted

Fix

Ensure allocations are HOST_COHERENT or use VMA flags intended for sequential writes, and/or call vmaFlushAllocation after writes.

Prefer VMA_MEMORY_USAGE_AUTO + VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT and keep a tiny helper:

MappedRange::write(ptr, size) that flushes only if needed.

2) Swapchain-image hazard: no per-image in-flight tracking

You fence per frame slot, not per swapchain image. If swapchain has more images than frames-in-flight, the same image_index can come back while the previous use of that image is still in flight unless you track and wait on an “images_in_flight[image] fence”. Your current flow waits only on frame.in_flight_fence() (slot fence). 

pasted

Fix

Keep std::vector<VkFence> images_in_flight sized to swapchain image count.

After acquire: if images_in_flight[image_index] != VK_NULL_HANDLE, wait it; then set it to this frame’s fence.

This is a common correctness bug that shows up under load / present modes / variable swapchain sizes.

3) AsyncUploader is “async” but can silently become “submit-order dependent”

Right now, your “transfer queue” is actually the graphics queue (transfer_queue() returns graphics queue). That makes create_texture “accidentally correct” because queue submission order guarantees the upload happens before the first frame’s rendering. But if you later introduce a real transfer queue (you even hint at it), you’ll need explicit cross-queue synchronization (timeline waits or ownership transfers).

Fix

Model uploads as producing a GpuSignal::timeline(value) and make the graphics submission wait on it (timeline wait in vkQueueSubmit2 or binary semaphore bridging).

Also handle queue-family ownership transfers if transfer != graphics (your comment already acknowledges this). 

pasted

Medium-risk design/safety problems
4) Resource retirement callbacks can become “use-after-destroy” if ownership graph changes

Buffer and Image retirement lambdas capture VmaAllocator and Vulkan handles and execute later via GpuRetirementQueue. This is fine only if allocator/device outlive the retirement queue flush. Today your Renderer::~Renderer() flushes the retirement queue before Context is destroyed (member order helps), so you’re safe in current layout.

But this is fragile architecture: one refactor and you’ve got delayed callbacks calling into a dead allocator/device.

Fix

Make “device/allocator lifetime” explicit in the retirement system:

Store a std::shared_ptr<DeviceContext> (or intrusive refcount) inside resources and in the retirement queue so callbacks cannot outlive the device context.

Or require GpuRetirementQueue to be owned by Context and destroyed before device teardown, enforcing this invariant structurally.

5) GpuRetirementQueue callbacks can throw during teardown paths

collect() and flush_all() call ANKH_THROW_MSG if a callback is null. Throwing from cleanup/destructors is dangerous (terminate risk). 

pasted

Fix

Make GpuRetirementQueue functions noexcept.

Replace throws with logging + continue (or ANKH_ASSERT(false) in debug).

6) Swapchain depth format selection is not robust

find_depth_format() always returns VK_FORMAT_D32_SFLOAT without querying support. This will break on some platforms/drivers. 

pasted

Fix

Query vkGetPhysicalDeviceFormatProperties and choose from a preference list:

D32_SFLOAT, D32_SFLOAT_S8_UINT, D24_UNORM_S8_UINT (depending on needs).

7) Descriptor sets bind a single texture “forever”; no lifetime contract enforced

Frame contexts bake descriptor sets that reference textureView/textureSampler at construction. If you ever hot-reload textures or rebuild the texture, you must either re-write all frame descriptor sets or guarantee the old texture stays alive until frames complete. Currently it’s “implicitly OK” because texture is created once and never replaced.

Fix

Introduce a Bindless-ish indirection (descriptor indexing) or centralize descriptor writes and do “per-frame descriptor updates” with explicit retirement of replaced resources.

Architecture-level improvements (performance + maintainability)
A) Make the frame graph explicit (even if small)

Right now, “frame lifetime” is spread across:

per-slot fence/semaphores (FrameContext) 

pasted

GpuSerial completion tracking and retirement queue collection

swapchain recreation pipeline

This is workable, but it’s easy to regress.

Recommendation

Create a single FrameScheduler (or FrameSync) that owns:

fences + semaphores (graphics)

per-image fences

serial issuance/completion

retirement collection calls

Renderer should “ask” scheduler for FrameToken each frame; everything else uses that token.

B) Stronger types & invariants for “device-bound” objects

Many wrappers store raw VkDevice and assume it’s valid. This is okay but brittle.

Recommendation

Store a reference to a DeviceDispatch/DeviceContext object that owns VkDevice, allocator, queues, etc.

Move-only Vulkan resources then become impossible to construct without a live context, and impossible to destroy after context is gone.

C) Upload staging lifetime is clever, but make it a pattern

You retire the staging buffer after the uploader’s timeline reaches the ticket value (good).
Turn this into a reusable utility:

Uploader::submit_and_retire(std::move(staging), signal)

ensures consistent policy across buffers/images.

D) Data-oriented per-frame writes

Your object buffer is CPU_TO_GPU and fully rewritten each frame. Fine for now, but you’ll want:

persistent mapped ring buffers (one big buffer, per-frame slice)

alignment/offset management

fewer allocations and less descriptor churn

Quick “action list” (in order)

Add swapchain image in-flight fences tracking. 

pasted

Make host writes safe: ensure coherent or flush via VMA after memcpy. 

pasted

Make AsyncUploader produce a signal that graphics waits on (so dedicated transfer queue won’t break later).

Make retirement queue noexcept and tie it structurally to device lifetime.

Fix depth format selection. 
