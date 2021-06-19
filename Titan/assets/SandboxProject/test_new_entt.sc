Scene: Scene Name
Environment:
  AssetPath: assets/env/birchwood_4k.hdr
  Light:
    Direction: [-1, -1, -1]
    Radiance: [0, -1, 0]
    Multiplier: 0
Entities:
  - Entity: 8964302238552097403
    TagComponent:
      Tag: Cube1m
    TransformComponent:
      Position: [-0.0160422307, -24.9639072, 0.0825139731]
      Rotation: [-0, 0, -1, -4.37113883e-08]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets/meshes/Cube1m.fbx
    RigidbodyComponent:
      Is Kinematic: false
    BoxColliderComponent:
      Extents: [4, 1, 4]
  - Entity: 8309017390750727753
    TagComponent:
      Tag: Cube1m2
    TransformComponent:
      Position: [0, -29.9476852, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [46.3290672, 1, 39.6601753]
    MeshComponent:
      AssetPath: assets/meshes/Cube1m.fbx
    RigidbodyComponent:
      Is Kinematic: false
    BoxColliderComponent:
      Extents: [46.3300018, 2, 46.3300018]