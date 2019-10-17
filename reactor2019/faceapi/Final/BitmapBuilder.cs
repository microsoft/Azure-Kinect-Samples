using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace AzureKinectFaceApi
{
    class BitmapBuilder
    {
        public static BitmapSource GetRenderedBitmap(BitmapSource originalImage, IEnumerable<StatusRectangle> rectangles)
        {
            int width = originalImage?.PixelWidth ?? 640;
            int height = originalImage?.PixelWidth ?? 480;

            // Create a new bitmap to render to
            RenderTargetBitmap targetBitmap = new RenderTargetBitmap(
                        width,
                        height,
                        300, 300,
                        PixelFormats.Pbgra32);

            // Set up a drawing context
            DrawingVisual visual = new DrawingVisual();
            DrawingContext context = visual.RenderOpen();

            if (originalImage != null)
            {
                // Draw the original image as the background
                context.DrawImage(originalImage,
                    new Rect(0, 0, targetBitmap.Width, targetBitmap.Height));
            }

            // Determine the scale between Pixel coordinates and Drawing coordinates
            double scale = targetBitmap.Width / (double)targetBitmap.PixelWidth;

            // Draw the rectangles and text
            foreach (StatusRectangle rectangle in rectangles)
            {
                Rect renderLocation = rectangle.PixelRectangle;

                // Scale from pixel coordinates to drawing coordinates
                renderLocation.Scale(scale, scale);

                Brush brush = Brushes.Green;

                // Format the text
                FormattedText formatted = new FormattedText(rectangle.Text,
                    CultureInfo.CurrentCulture,
                    FlowDirection.LeftToRight,
                    new Typeface("Verdana"),
                    15.0,
                    brush,
                    1.0);

                // Draw the rectangle and text
                context.DrawRoundedRectangle(null, new Pen(brush, 2.0), renderLocation, 3.0, 3.0);
                context.DrawText(formatted, renderLocation.TopRight);
            }

            // Complete the render
            context.Close();
            targetBitmap.Render(visual);

            return targetBitmap;
        }
    }
}
