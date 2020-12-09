Scene: Scene Name
Environment:
  AssetPath: assets\env\birchwood_4k.hdr
  Light:
    Direction: [-1, -1, -1]
    Radiance: [0, -1, 0]
    Multiplier: 0
Entities:
  - Entity: 7514802698436192091
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 1, 0]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
  - Entity: 5909569925960781592
    TagComponent:
      Tag: Floor
    TransformComponent:
      Position: [0, -10, 0]
      Rotation: [0, -0, 0, 1]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets\models\Plane1m.fbx