using System;

namespace SaturnBuildTool
{
    // Valid arguments for SPROPERTY macro
    [Flags]
    public enum SP
    {
        None = 0,
        EditInEditor = 1 << 0,
        ReadOnlyInEditor = 1 << 1,
        Asset = 1 << 2,
        Serialised = 1 << 3
    }

    public enum SPType 
    {
        Char,
        Float,
        Int,
        Double,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Int8,
        Int16,
        Int32, /* same as int*/
        Int64,
        Vector3, /* glm::vec2 */
        Vector2, /* glm::vec3 */
        Vector4, /* glm::vec4 */
        String, /* std::string */
        Object,
        AssetHandle,
        Class,
        Unknown
    }

    public struct SProperty
    {
        public string Name;
        public SPType Type;

        public int Line;
        public SP Flags;
    }
}
