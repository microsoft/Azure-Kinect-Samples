using OpenGL;
using System;
using System.Linq;

namespace Csharp_3d_viewer
{
    public class TriangleRenderer : VertexRenderer
    {
        private readonly uint elementBufferObject;
        private int[] indices;

        public TriangleRenderer()
        {
            // Generate OpenGL buffer for storing vertex indices defining face triangles.
            elementBufferObject = Gl.GenBuffer();
        }

        internal void UpdateTriangles(Vertex[] vertices, int[] indices)
        {
            // Perform sanity check on indices array.
            if ((indices.Length % 3 != 0) ||
                indices.Any(index => (index < 0) || (index > vertices.Length - 1)))
            {
                throw new Exception("Array of triangle indices is invalid.");
            }

            this.indices = indices;

            UpdateVertices(vertices);

            Gl.BindVertexArray(vertexArrayObject);
            Gl.BindBuffer(BufferTarget.ElementArrayBuffer, elementBufferObject);
            Gl.BufferData(BufferTarget.ElementArrayBuffer, (uint)(sizeof(int) * indices.Length), indices, BufferUsage.StaticDraw);
        }

        protected override void DrawElements()
        {
            Gl.BindVertexArray(vertexArrayObject);
            Gl.DrawElements(PrimitiveType.Triangles, indices.Length, DrawElementsType.UnsignedInt, IntPtr.Zero);
        }
    }
}
