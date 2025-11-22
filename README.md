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
