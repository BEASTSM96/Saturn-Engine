Scene: Scene Name
Environment:
  AssetPath: assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [-1, -1, -1]
    Radiance: [0, -1, 0]
    Multiplier: 0
Entities:
  - Entity: 14492712895718713112
    TagComponent:
      Tag: Sky Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0, 0]
      Scale: [1, 1, 1]
    SkyLightComponent:
      Environment File path: assets\env\pink_sunrise_4k.hdr
  - Entity: 12069022435483687739
    TagComponent:
      Tag: Floor
    TransformComponent:
      Position: [0, -0.592308044, 0]
      Rotation: [0, 0, 0, 0]
      Scale: [35.9775581, 1.04284251, 34.7892838]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: true
      Mass: 0
      CCD: true
      Material-DynamicFriction: 1
      Material-StaticFriction: 1
      Material-Restitution: 1
    PhysXBoxColliderComponent:
      Extents: [1, 1, 1]
  - Entity: 6897180264170162680
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0, 15.9208851, 0]
      Rotation: [0, 0, 0, 0]
      Scale: [5.00999928, 5.00999928, 5.00999928]
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: false
      Mass: 2
      CCD: true
      Material-DynamicFriction: 1
      Material-StaticFriction: 1
      Material-Restitution: 1
    PhysXSphereColliderComponent:
      Radius: 1