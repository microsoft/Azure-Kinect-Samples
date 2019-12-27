using OpenGL;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace Csharp_3d_viewer
{
    public class PointCloudRenderer : VertexRenderer
    {
        private const float pointSize = 3.0f;

        public void Render(IEnumerable<Vertex> points, Vector4 color)
        {
            UpdateVertices(points.ToArray());
            Render(Matrix4x4.Identity, color);
        }

        protected override void DrawElements()
        {
            Gl.Enable(EnableCap.DepthTest);

            // Enable blending
            Gl.Enable(EnableCap.Blend);
            Gl.BlendFunc(BlendingFactor.SrcAlpha, BlendingFactor.OneMinusSrcAlpha);

            Gl.PointSize(pointSize);

            Gl.BindVertexArray(vertexArrayObject);
            Gl.DrawArrays(PrimitiveType.Points, 0, Vertices.Length);
        }
    }
}
