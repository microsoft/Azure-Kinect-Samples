using System.Linq;

namespace Csharp_3d_viewer
{
    public static class Shaders
    {
        private const string VertexShaderText = @"
            #version 430
            layout(location = 0) in vec3 vertexPosition;
            layout(location = 1) in vec3 vertexNormal;

            varying vec4 fragmentColor;
            varying vec3 fragmentPosition;
            varying vec3 fragmentNormal;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform vec4 color;

            void main()
            {
                fragmentColor = color;
                fragmentPosition = vec3(model * vec4(vertexPosition, 1.0));
                fragmentNormal = mat3(transpose(inverse(model))) * vertexNormal;

                gl_Position = projection * view * model * vec4(vertexPosition, 1);
            }
        ";

        private const string FragmentShaderText = @"
            #version 430
            varying vec4 fragmentColor;
            varying vec3 fragmentPosition;
            varying vec3 fragmentNormal;

            void main()
            {
                vec3 lightPosition = vec3(0, 0, 0);

                // diffuse
                vec3 norm = normalize(fragmentNormal);
                vec3 lightDir = lightPosition - fragmentPosition;
                float dist2 = dot(lightDir, lightDir);
                lightDir = normalize(lightDir);
                float diffuse = abs(dot(norm, lightDir)) / dist2;

                gl_FragColor = vec4(fragmentColor.rgb * diffuse, fragmentColor.a);
            }
        ";

        public static string[] VertexShaderLines => ConvertToLines(VertexShaderText);
        public static string[] VertexFragmentLines => ConvertToLines(FragmentShaderText);

        private static string[] ConvertToLines(string text)
        {
            return text.Split('\n').Select(s => s + '\n').ToArray();
        }
    }
}
