using System;
using System.Collections.Generic;
using System.Numerics;

namespace Csharp_3d_viewer
{
    public class SphereRenderer : TriangleRenderer
    {
        private const float radius = 1.0f;
        private const int MinSectorCount = 3;
        private const int MinStackCount = 2;

        public SphereRenderer(int sectorCount = 36, int stackCount = 18)
        {
            sectorCount = Math.Max(sectorCount, MinSectorCount);
            stackCount = Math.Max(stackCount, MinStackCount);

            BuildVertices(sectorCount, stackCount, out var vertices, out var indices);
            UpdateTriangles(vertices, indices);
        }

        public void Render(Vector3 position, float radius, Vector4 color)
        {
            var model = Matrix4x4.CreateScale(radius) * Matrix4x4.CreateTranslation(position);
            Render(model, color);
        }

        private void BuildVertices(int sectorCount, int stackCount, out Vertex[] vertices, out int[] indices)
        {
            var radiusInv = 1.0f / radius;
            var sectorStep = (float)(2 * Math.PI / sectorCount);
            var stackStep = Math.PI / stackCount;

            var sphereVertices = new List<Vertex>();
            for (int i = 0; i <= stackCount; ++i)
            {
                Vector3 position = new Vector3();
                var stackAngle = (float)(Math.PI / 2 - i * stackStep);
                var xy = radius * Math.Cos(stackAngle);
                position.Z = (float)(radius * Math.Sin(stackAngle));

                for (int j = 0; j <= sectorCount; ++j)
                {
                    var sectorAngle = j * sectorStep;

                    position.X = (float)(xy * Math.Cos(sectorAngle));
                    position.Y = (float)(xy * Math.Sin(sectorAngle));

                    Vector3 normal = new Vector3(position.X, position.Y, position.Z) * radiusInv;

                    sphereVertices.Add(new Vertex { Position = position, Normal = normal });
                }
            }

            vertices = sphereVertices.ToArray();

            var sphereIndices = new List<int>();
            for (int i = 0; i < stackCount; ++i)
            {
                var k1 = i * (sectorCount + 1);
                var k2 = k1 + sectorCount + 1;

                for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
                {
                    if (i != 0)
                    {
                        sphereIndices.Add(k1);
                        sphereIndices.Add(k2);
                        sphereIndices.Add(k1 + 1);
                    }
                    if (i != (stackCount - 1))
                    {
                        sphereIndices.Add(k1 + 1);
                        sphereIndices.Add(k2);
                        sphereIndices.Add(k2 + 1);
                    }
                }
            }
            indices = sphereIndices.ToArray();
        }
    }
}
