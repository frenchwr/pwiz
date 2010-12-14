﻿using pwiz.Topograph.ui.Controls;

namespace pwiz.Topograph.ui.Forms
{
    partial class ChromatogramForm
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
            this.gridIntensities = new pwiz.Topograph.ui.Controls.ExcludedMzsGrid();
            this.cbxOverrideExcludedMzs = new System.Windows.Forms.CheckBox();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.cbxAutoFindPeak = new System.Windows.Forms.CheckBox();
            this.cbxOverrideExcludedMasses = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.gridIntensities)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // gridIntensities
            // 
            this.gridIntensities.AllowUserToAddRows = false;
            this.gridIntensities.AllowUserToDeleteRows = false;
            this.tableLayoutPanel1.SetColumnSpan(this.gridIntensities, 2);
            this.gridIntensities.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gridIntensities.Location = new System.Drawing.Point(3, 53);
            this.gridIntensities.Name = "gridIntensities";
            this.gridIntensities.PeptideAnalysis = null;
            this.gridIntensities.PeptideFileAnalysis = null;
            this.gridIntensities.Size = new System.Drawing.Size(291, 208);
            this.gridIntensities.TabIndex = 11;
            // 
            // cbxOverrideExcludedMzs
            // 
            this.cbxOverrideExcludedMzs.AutoSize = true;
            this.cbxOverrideExcludedMzs.Location = new System.Drawing.Point(3, 28);
            this.cbxOverrideExcludedMzs.Name = "cbxOverrideExcludedMzs";
            this.cbxOverrideExcludedMzs.Size = new System.Drawing.Size(134, 17);
            this.cbxOverrideExcludedMzs.TabIndex = 13;
            this.cbxOverrideExcludedMzs.Text = "Override excluded Mzs";
            this.cbxOverrideExcludedMzs.UseVisualStyleBackColor = true;
            this.cbxOverrideExcludedMzs.CheckedChanged += new System.EventHandler(this.cbxOverrideExcludedMzs_CheckedChanged);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.tableLayoutPanel1);
            this.splitContainer1.Size = new System.Drawing.Size(893, 264);
            this.splitContainer1.SplitterDistance = 297;
            this.splitContainer1.TabIndex = 3;
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 100F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.cbxAutoFindPeak, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.cbxOverrideExcludedMasses, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.gridIntensities, 0, 2);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 3;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 25F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 25F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(297, 264);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // cbxAutoFindPeak
            // 
            this.cbxAutoFindPeak.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.cbxAutoFindPeak, 2);
            this.cbxAutoFindPeak.Location = new System.Drawing.Point(3, 3);
            this.cbxAutoFindPeak.Name = "cbxAutoFindPeak";
            this.cbxAutoFindPeak.Size = new System.Drawing.Size(99, 17);
            this.cbxAutoFindPeak.TabIndex = 12;
            this.cbxAutoFindPeak.Text = "Auto Find Peak";
            this.cbxAutoFindPeak.UseVisualStyleBackColor = true;
            this.cbxAutoFindPeak.CheckedChanged += new System.EventHandler(this.cbxAutoFindPeak_CheckedChanged);
            // 
            // cbxOverrideExcludedMasses
            // 
            this.cbxOverrideExcludedMasses.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.cbxOverrideExcludedMasses, 2);
            this.cbxOverrideExcludedMasses.Location = new System.Drawing.Point(3, 28);
            this.cbxOverrideExcludedMasses.Name = "cbxOverrideExcludedMasses";
            this.cbxOverrideExcludedMasses.Size = new System.Drawing.Size(150, 17);
            this.cbxOverrideExcludedMasses.TabIndex = 13;
            this.cbxOverrideExcludedMasses.Text = "Override excluded masses";
            this.cbxOverrideExcludedMasses.UseVisualStyleBackColor = true;
            this.cbxOverrideExcludedMasses.CheckedChanged += new System.EventHandler(this.cbxOverrideExcludedMasses_CheckedChanged);
            // 
            // ChromatogramForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(893, 264);
            this.Controls.Add(this.splitContainer1);
            this.Name = "ChromatogramForm";
            this.Text = "Raw Chromatograms";
            this.ResizeEnd += new System.EventHandler(this.ChromatogramForm_ResizeEnd);
            this.Resize += new System.EventHandler(this.ChromatogramForm_Resize);
            ((System.ComponentModel.ISupportInitialize)(this.gridIntensities)).EndInit();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

        }
        protected ExcludedMzsGrid gridIntensities;
        private System.Windows.Forms.CheckBox cbxOverrideExcludedMzs;

        #endregion
        private System.Windows.Forms.SplitContainer splitContainer1;
        protected System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.CheckBox cbxAutoFindPeak;
        private System.Windows.Forms.CheckBox cbxOverrideExcludedMasses;
    }
}