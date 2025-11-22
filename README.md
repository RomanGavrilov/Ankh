# Ankh
platform
  ↓
core (Instance/Device/Queues/Debug)
  ↓
swapchain → renderpass → pipeline
       ↘         ↘           ↘
        memory ← descriptors  ↘
            ↘      ↘          commands
             sync   ↘            ↘
                    frame  ←──────┘
                       ↓
                    renderer
                       ↓
                     app

1. device.hpp forward declares class PhysicalDevice;
