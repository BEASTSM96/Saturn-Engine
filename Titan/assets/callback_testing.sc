Scene: Scene Name
Environment:
  AssetPath: assets\env\birchwood_4k.hdr
  Light:
    Direction: [-1, -1, -1]
    Radiance: [0, -1, 0]
    Multiplier: 0
Entities:
  - Entity: 173420204987980016
    TagComponent:
      Tag: Empty Entity
    TransformComponent:
      Position: [0, 14.1658697, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [9.38999939, 9.38999939, 9.38999939]
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: false
    PhysXSphereColliderComponent:
      Radius: 4.69499969
  - Entity: 10950120370714488226
    TagComponent:
      Tag: Empty Entity
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [35.2174225, 1, 32.2669983]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: true
    PhysXBoxColliderComponent:
      Extents: [17.6087112, 0.5, 16.1334991]