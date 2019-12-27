using OpenGL;
using System;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;

namespace Csharp_3d_viewer
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vertex
    {
        public Vector3 Position;
        public Vector3 Normal;
    };

    public abstract class VertexRenderer
    {
        private readonly uint vertexBufferObject;
        protected readonly uint vertexArrayObject;

        private uint shaderProgram;
        private int shaderModelIndex;
        private int shaderViewIndex;
        private int shaderProjectionIndex;
        private int shaderColorIndex;

        internal VertexRenderer()
        {
            View = Matrix4x4.Identity;
            Projection = Matrix4x4.Identity;

            // Generate vertex buffers.
            vertexArrayObject = Gl.GenVertexArray();
            vertexBufferObject = Gl.GenBuffer();

            CreateProgram();
        }

        public Matrix4x4 View { get; set; }
        public Matrix4x4 Projection { get; set; }

        public Vertex[] Vertices { get; private set; }

        public void Render(Matrix4x4 model, Vector4 color)
        {
            Gl.UseProgram(shaderProgram);

            // Set shader variables based on current render parameters.
            Gl.UniformMatrix4f(shaderViewIndex, 1, false, View);
            Gl.UniformMatrix4f(shaderProjectionIndex, 1, false, Projection);
            Gl.Uniform4f(shaderColorIndex, 1, color);
            Gl.UniformMatrix4f(shaderModelIndex, 1, false, model);

            DrawElements();
        }

        protected void UpdateVertices(Vertex[] vertices)
        {
            this.Vertices = vertices;

            var vertexSize = Marshal.SizeOf<Vertex>();
            Gl.BindVertexArray(vertexArrayObject);
            Gl.BindBuffer(BufferTarget.ArrayBuffer, vertexBufferObject);
            Gl.BufferData(BufferTarget.ArrayBuffer, (uint)(vertexSize * vertices.Length), vertices, BufferUsage.StreamDraw);

            // Set the vertex attribute pointers.
            Gl.EnableVertexAttribArray(0);
            Gl.VertexAttribPointer(0, 3, VertexAttribType.Float, false, vertexSize, IntPtr.Zero);
            Gl.EnableVertexAttribArray(1);
            Gl.VertexAttribPointer(1, 3, VertexAttribType.Float, false, vertexSize, new IntPtr(vertexSize / 2));
        }

        protected abstract void DrawElements();

        private void CreateProgram()
        {
            StringBuilder infolog = new StringBuilder(1024);

            // Vertex shader
            uint vertexShader = Gl.CreateShader(ShaderType.VertexShader);
            Gl.ShaderSource(vertexShader, Shaders.VertexShaderLines);
            Gl.CompileShader(vertexShader);
            Gl.GetShader(vertexShader, ShaderParameterName.CompileStatus, out int compiled);
            if (compiled == 0)
            {
                Gl.GetShaderInfoLog(vertexShader, infolog.Capacity, out _, infolog);
                throw new Exception($"Compilation error:\n{infolog.ToString()}");
            }

            // Fragment shader
            uint fragmentShader = Gl.CreateShader(ShaderType.FragmentShader);
            Gl.ShaderSource(fragmentShader, Shaders.VertexFragmentLines);
            Gl.CompileShader(fragmentShader);
            Gl.GetShader(fragmentShader, ShaderParameterName.CompileStatus, out compiled);
            if (compiled == 0)
            {
                Gl.GetShaderInfoLog(fragmentShader, infolog.Capacity, out _, infolog);
                throw new Exception($"Compilation error:\n{infolog.ToString()}");
            }

            // Program
            shaderProgram = Gl.CreateProgram();
            Gl.AttachShader(shaderProgram, vertexShader);
            Gl.AttachShader(shaderProgram, fragmentShader);
            Gl.LinkProgram(shaderProgram);

            Gl.GetProgram(shaderProgram, ProgramProperty.LinkStatus, out int linked);

            if (linked == 0)
            {
                Gl.GetProgramInfoLog(shaderProgram, infolog.Capacity, out _, infolog);
                throw new Exception($"Linking error:\n{infolog.ToString()}");
            }

            shaderModelIndex = Gl.GetUniformLocation(shaderProgram, "model");
            shaderViewIndex = Gl.GetUniformLocation(shaderProgram, "view");
            shaderProjectionIndex = Gl.GetUniformLocation(shaderProgram, "projection");
            shaderColorIndex = Gl.GetUniformLocation(shaderProgram, "color");
        }
    }
}
