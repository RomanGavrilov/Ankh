Renderer
 └─ Frame
     └─ RenderPass (VkRenderPass)
         ├─ DrawPass   ← main geometry
         └─ UiPass     ← overlay / debug / imgui

This structure is exactly what enables:

shadow pass

depth pre-pass

forward+ light pass

post-process pass

UI pass

debug visualization pass
