using System;
using System.Collections.Generic;
using System.Numerics;

namespace Csharp_3d_viewer
{
    public class CylinderRenderer : TriangleRenderer
    {
        private const float baseRadius = 1.0f;
        private const float height = 1.0f;
        private const float baseScale = 0.01f;
        private const int MinSectorCount = 3;

        public CylinderRenderer(int sectorCount = 36)
        {
            sectorCount = Math.Max(sectorCount, MinSectorCount);

            BuildVertices(sectorCount, out var vertices, out var indices);
            UpdateTriangles(vertices, indices);
        }

        public void Render(Vector3 start, Vector3 end, Vector4 color)
        {
            var centralAxis = start - end;
            var length = centralAxis.Length();
            var centerPosition = (start + end) / 2;
            var model = Matrix4x4.CreateScale(baseScale, baseScale, length) * Matrix4x4.CreateWorld(centerPosition, -centralAxis, Vector3.UnitZ);

            Render(model, color);
        }

        private void BuildVertices(int sectorCount, out Vertex[] vertices, out int[] indices)
        {
            var radiusInv = 1.0f / baseRadius;
            var sectorStep = (float)(2 * Math.PI / sectorCount);

            var verticesList = new List<Vertex>();
            for (int circleIndex = 0; circleIndex < 2; ++circleIndex)
            {
                Vector3 position = new Vector3();
                position.Z = height / 2 * (1 - 2 * circleIndex);

                for (int j = 0; j <= sectorCount; ++j)
                {
                    var sectorAngle = j * sectorStep;

                    position.X = (float)(baseRadius * Math.Cos(sectorAngle));
                    position.Y = (float)(baseRadius * Math.Sin(sectorAngle));

                    Vector3 normal = new Vector3(position.X, position.Y, 0.0f) * radiusInv;

                    verticesList.Add(new Vertex { Position = position, Normal = normal });
                }
            }

            vertices = verticesList.ToArray();

            var indicesList = new List<int>();
            var k1 = 0;
            var k2 = k1 + sectorCount + 1;

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                indicesList.Add(k1);
                indicesList.Add(k2);
                indicesList.Add(k1 + 1);
                indicesList.Add(k1 + 1);
                indicesList.Add(k2);
                indicesList.Add(k2 + 1);
            }

            indices = indicesList.ToArray();
        }
    }
}