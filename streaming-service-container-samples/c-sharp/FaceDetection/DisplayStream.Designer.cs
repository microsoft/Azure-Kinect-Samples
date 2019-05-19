using System.Drawing;

namespace FaceDetection
{
    partial class DisplayStream
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.displayBox = new System.Windows.Forms.PictureBox();
            this.mainStatusStrip = new System.Windows.Forms.StatusStrip();
            this.statusServiceUrlLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusServiceUrl = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusSpringLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusDistance = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            ((System.ComponentModel.ISupportInitialize)(this.displayBox)).BeginInit();
            this.mainStatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // displayBox
            // 
            this.displayBox.BackColor = System.Drawing.SystemColors.Control;
            this.displayBox.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.displayBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.displayBox.Location = new System.Drawing.Point(0, 0);
            this.displayBox.Margin = new System.Windows.Forms.Padding(4);
            this.displayBox.Name = "displayBox";
            this.displayBox.Size = new System.Drawing.Size(3168, 1437);
            this.displayBox.TabIndex = 2;
            this.displayBox.TabStop = false;
            // 
            // mainStatusStrip
            // 
            this.mainStatusStrip.ImageScalingSize = new System.Drawing.Size(32, 32);
            this.mainStatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusServiceUrlLabel,
            this.statusServiceUrl,
            this.statusSpringLabel,
            this.statusDistance});
            this.mainStatusStrip.Location = new System.Drawing.Point(0, 1395);
            this.mainStatusStrip.Name = "mainStatusStrip";
            this.mainStatusStrip.Padding = new System.Windows.Forms.Padding(2, 0, 28, 0);
            this.mainStatusStrip.Size = new System.Drawing.Size(3168, 42);
            this.mainStatusStrip.TabIndex = 4;
            this.mainStatusStrip.Text = "statusStrip1";
            // 
            // statusServiceUrlLabel
            // 
            this.statusServiceUrlLabel.Name = "statusServiceUrlLabel";
            this.statusServiceUrlLabel.Padding = new System.Windows.Forms.Padding(40, 0, 0, 0);
            this.statusServiceUrlLabel.Size = new System.Drawing.Size(303, 37);
            this.statusServiceUrlLabel.Text = "Cognitive Services URL:";
            // 
            // statusServiceUrl
            // 
            this.statusServiceUrl.Name = "statusServiceUrl";
            this.statusServiceUrl.Size = new System.Drawing.Size(0, 37);
            // 
            // statusSpringLabel
            // 
            this.statusSpringLabel.Name = "statusSpringLabel";
            this.statusSpringLabel.Size = new System.Drawing.Size(2688, 37);
            this.statusSpringLabel.Spring = true;
            // 
            // statusDistance
            // 
            this.statusDistance.Font = new System.Drawing.Font("Segoe UI", 10F);
            this.statusDistance.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(120)))), ((int)(((byte)(212)))));
            this.statusDistance.Name = "statusDistance";
            this.statusDistance.Size = new System.Drawing.Size(85, 37);
            this.statusDistance.Text = "0 mm";
            // 
            // menuStrip1
            // 
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(32, 32);
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(3168, 24);
            this.menuStrip1.TabIndex = 7;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // DisplayStream
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(12F, 25F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(3168, 1437);
            this.Controls.Add(this.mainStatusStrip);
            this.Controls.Add(this.menuStrip1);
            this.Controls.Add(this.displayBox);
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "DisplayStream";
            this.Text = "Azure Kinect - Face Recognition";
            ((System.ComponentModel.ISupportInitialize)(this.displayBox)).EndInit();
            this.mainStatusStrip.ResumeLayout(false);
            this.mainStatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox displayBox;
        private System.Windows.Forms.StatusStrip mainStatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel statusServiceUrlLabel;
        private System.Windows.Forms.ToolStripStatusLabel statusServiceUrl;
        private System.Windows.Forms.ToolStripStatusLabel statusSpringLabel;
        private System.Windows.Forms.ToolStripStatusLabel statusDistance;
        private System.Windows.Forms.MenuStrip menuStrip1;
    }
}