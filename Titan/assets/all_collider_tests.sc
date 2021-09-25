Scene: Scene Name
Environment:
  AssetPath: assets\env\birchwood_4k.hdr
  Light:
    Direction: [-1, -1, -1]
    Radiance: [0, -1, 0]
    Multiplier: 0
Entities:
  - Entity: 8378351155750265526
    TagComponent:
      Tag: Cube
    TransformComponent:
      Position: [-8.03676987, 20.4025898, -9.44089031]
      Rotation: [0, 0, 0, 1]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: false
      Mass: 1
      CCD: true
    PhysXBoxColliderComponent:
      Extents: [0.5, 0.5, 0.5]
  - Entity: 3055790865195180191
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0, 20.3732986, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: false
      Mass: 1
      CCD: true
    PhysXSphereColliderComponent:
      Radius: 0.5
  - Entity: 2320747535667194579
    TagComponent:
      Tag: Floor
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0, 1]
      Scale: [34.5991325, 0.330227077, 32.6803017]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    PhysXRigidbodyComponent:
      Kinematic: true
    PhysXBoxColliderComponent:
      Extents: [17.2995663, 0.165113539, 16.3401508]