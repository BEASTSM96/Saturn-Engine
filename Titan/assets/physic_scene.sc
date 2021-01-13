Scene: Scene Name
Environment:
  AssetPath: assets/env/birchwood_4k.hdr
  Light:
    Direction: [-1, -1, -1]
    Radiance: [0, -1, 0]
    Multiplier: 0
Entities:
  - Entity: 8964301118772097403
    TagComponent:
      Tag: Cube1m2
    TransformComponent:
      Position: [0, -30, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [32.1300011, 1, 32.1300011]
    MeshComponent:
      AssetPath: assets/meshes/Cube1m.fbx
    RigidbodyComponent:
      Is Kinematic: true
    BoxColliderComponent:
      Extents: [32.1300011, 1, 31.4200001]
  - Entity: 8964302238552097403
    TagComponent:
      Tag: Cube1m
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets/meshes/Cube1m.fbx
    RigidbodyComponent:
      Is Kinematic: false
    BoxColliderComponent:
      Extents: [4, 1, 4]