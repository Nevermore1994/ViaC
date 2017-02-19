namespace viacode
{
    partial class Find
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
                components.Dispose( );
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Find));
            this.findText = new System.Windows.Forms.RichTextBox();
            this.findbutton = new System.Windows.Forms.Button();
            this.findallbutton = new System.Windows.Forms.Button();
            this.replacebutton = new System.Windows.Forms.Button();
            this.selectBox = new System.Windows.Forms.ComboBox();
            this.selectModel = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // findText
            // 
            this.findText.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.findText.Location = new System.Drawing.Point(30, 30);
            this.findText.Name = "findText";
            this.findText.Size = new System.Drawing.Size(403, 36);
            this.findText.TabIndex = 0;
            this.findText.Text = "";
            // 
            // findbutton
            // 
            this.findbutton.Location = new System.Drawing.Point(30, 170);
            this.findbutton.Name = "findbutton";
            this.findbutton.Size = new System.Drawing.Size(130, 46);
            this.findbutton.TabIndex = 2;
            this.findbutton.Text = "查找";
            this.findbutton.UseVisualStyleBackColor = true;
            // 
            // findallbutton
            // 
            this.findallbutton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.findallbutton.Location = new System.Drawing.Point(300, 170);
            this.findallbutton.Name = "findallbutton";
            this.findallbutton.Size = new System.Drawing.Size(130, 46);
            this.findallbutton.TabIndex = 3;
            this.findallbutton.Text = "查找所有";
            this.findallbutton.UseVisualStyleBackColor = true;
            // 
            // replacebutton
            // 
            this.replacebutton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.replacebutton.Location = new System.Drawing.Point(463, 30);
            this.replacebutton.Name = "replacebutton";
            this.replacebutton.Size = new System.Drawing.Size(39, 36);
            this.replacebutton.TabIndex = 4;
            this.replacebutton.UseVisualStyleBackColor = true;
            this.replacebutton.Click += new System.EventHandler(this.replacebutton_Click);
            // 
            // selectBox
            // 
            this.selectBox.AutoCompleteCustomSource.AddRange(new string[] {
            "下一个",
            "上一个"});
            this.selectBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.selectBox.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.selectBox.FormattingEnabled = true;
            this.selectBox.ItemHeight = 20;
            this.selectBox.Items.AddRange(new object[] {
            "下一个",
            "上一个"});
            this.selectBox.Location = new System.Drawing.Point(30, 124);
            this.selectBox.Name = "selectBox";
            this.selectBox.Size = new System.Drawing.Size(121, 28);
            this.selectBox.TabIndex = 5;
            // 
            // selectModel
            // 
            this.selectModel.AutoCompleteCustomSource.AddRange(new string[] {
            "非全字匹配",
            "全字匹配"});
            this.selectModel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.selectModel.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.selectModel.FormattingEnabled = true;
            this.selectModel.ItemHeight = 20;
            this.selectModel.Items.AddRange(new object[] {
            "非全字匹配",
            "全字匹配"});
            this.selectModel.Location = new System.Drawing.Point(300, 124);
            this.selectModel.Name = "selectModel";
            this.selectModel.Size = new System.Drawing.Size(121, 28);
            this.selectModel.TabIndex = 6;
            // 
            // Find
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(532, 253);
            this.Controls.Add(this.selectModel);
            this.Controls.Add(this.selectBox);
            this.Controls.Add(this.replacebutton);
            this.Controls.Add(this.findallbutton);
            this.Controls.Add(this.findbutton);
            this.Controls.Add(this.findText);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.Name = "Find";
            this.Text = "查找";
            this.Load += new System.EventHandler(this.Find_Load);
            this.ResumeLayout(false);

        }

        #endregion

        public System.Windows.Forms.Button replacebutton;
        public System.Windows.Forms.RichTextBox findText;
        public System.Windows.Forms.Button findbutton;
        public System.Windows.Forms.Button findallbutton;
        public System.Windows.Forms.ComboBox selectBox;
        public System.Windows.Forms.ComboBox selectModel;
    }
}