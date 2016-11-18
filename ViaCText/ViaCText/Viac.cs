using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ViaCText
{
    public partial class Viac : Form
    {
        string path = null;
        string isSaved = "N";
        int initLengIt = 0;
        string textstyle = "unicode";
        
        public Viac()
        {
            InitializeComponent( );
        }
        private void Viac_Load(object sender, EventArgs e)
        {
            复制ToolStripMenuItem.Enabled = false;
            粘贴ToolStripMenuItem.Enabled = false;
            if (!string.IsNullOrEmpty(path))
                OpenFile(".ViaC");
        }

        private void OpenFile(string style)
        {
            try
            {
                string extname = path.Substring(path.LastIndexOf("."));
                if(extname.ToLower().Equals(style))
                {
                    if(textstyle.Equals("unicode"))
                     richTextBox1.LoadFile(path, RichTextBoxStreamType.UnicodePlainText);
                    else
                        richTextBox1.LoadFile(path, RichTextBoxStreamType.PlainText);
                }
                Text = "ViaC编译器" + path;
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString(), "提示");
            } 
        }

        private void ubicodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            textstyle = "unicode";
            ubicodeToolStripMenuItem.Checked = true;
            acsiiToolStripMenuItem.Checked = false;
            if(!string.IsNullOrEmpty(path))
                richTextBox1.LoadFile(path, RichTextBoxStreamType.UnicodePlainText);
        }

        private void acsiiToolStripMenuItem_Click(object sender, EventArgs e)
        {
            textstyle = "ascii";
            ubicodeToolStripMenuItem.Checked = false;
            acsiiToolStripMenuItem.Checked = true;
            if (!string.IsNullOrEmpty(path))
                richTextBox1.LoadFile(path, RichTextBoxStreamType.PlainText);
        }

        private void viaC文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.openFileDialog1.ShowDialog( ) == DialogResult.OK)
            {
                path = this.openFileDialog1.FileName;
                OpenFile(".viac");
            }
        }

        private void 头文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.openFileDialog1.ShowDialog( ) == DialogResult.OK)
            {
                path = this.openFileDialog1.FileName;
                OpenFile(".h");
            }
        }

        private void 其他ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.openFileDialog1.ShowDialog( ) == DialogResult.OK)
            {
                path = this.openFileDialog1.FileName;
                OpenFile(".txt");
            }
        }

        private void 保存ToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(path))
            {
                saveFileDialog1.Title = "保存为";
                if (saveFileDialog1.ShowDialog( ) == DialogResult.OK)
                {
                    path = saveFileDialog1.FileName;
                    SaveFile( );
                } 
            }
            else
            {
                string extname = path.Substring(path.LastIndexOf("."));
                if(extname.ToLower().Equals(".txt") || extname.ToLower( ).Equals(".viac")||
                    extname.ToLower( ).Equals(".c") || extname.ToLower( ).Equals(".c"))
                {
                    if (textstyle.Equals("ascii"))
                        richTextBox1.SaveFile( path, RichTextBoxStreamType.RichText);
                    else
                        richTextBox1.SaveFile(path, RichTextBoxStreamType.UnicodePlainText);
                }
            }
        }
        private void SaveFile()
        {
            try
            {
                if (textstyle.Equals("ascii"))
                    richTextBox1.SaveFile(path, RichTextBoxStreamType.RichText);
                else
                    richTextBox1.SaveFile(path, RichTextBoxStreamType.UnicodePlainText);
            }
            catch
            {
                MessageBox.Show("保存失败","提示");
            }
           
        }

       

        private void 新建ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if( isSaved.Equals("N"))
            {
                 DialogResult res = MessageBox.Show("文件没有保存", "提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if ( res == DialogResult.Yes)
                {
                    richTextBox1.Clear( );
                    initLengIt = richTextBox1.TextLength;
                    保存ToolStripMenuItem1_Click(sender, e);
                }
            }
            else
            {
                richTextBox1.Clear( );
                Text = "ViaC编译器";
                initLengIt = richTextBox1.TextLength;
            }
        }

        private void 另存为ToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            保存ToolStripMenuItem1_Click(sender, e);
        }

        private void 复制ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            richTextBox1.Copy( );
        }
        private void 粘贴ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            richTextBox1.Paste( );
        }
        private void 剪切ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            richTextBox1.Cut( );
        }
        private void 撤销ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            richTextBox1.Undo( );
        }

        private void 删除ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (richTextBox1.SelectedText != "")
            {
                richTextBox1.SelectedText = "";
            }
        }
        private void 重做ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            richTextBox1.Redo( );
        }
        private void 全选ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            richTextBox1.HideSelection = false;
            richTextBox1.SelectAll( );
        }
        private void 字体ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SetFont( );
            }
            catch
            {
                MessageBox.Show("设置字体失败", "错误提示");
            }
        }
        private void SetFont()
        {
            if(fontDialog1.ShowDialog() == DialogResult.OK)
            {
                richTextBox1.Font = fontDialog1.Font;
            }
        }
        private void 复制本行ToolStripMenuItem_Click(object sender, EventArgs e)
        {
             
        }
        private void 颜色ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(colorDialog1.ShowDialog() == DialogResult.OK)
            {
                richTextBox1.SelectionColor = colorDialog1.Color;
            }
        }

        private void 退出ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if ( (isSaved.Equals("N")))
            {
                DialogResult res = MessageBox.Show("文件没有保存", "提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if (res == DialogResult.Yes)
                {
                    richTextBox1.Clear( );
                    initLengIt = richTextBox1.TextLength;
                    保存ToolStripMenuItem1_Click(sender, e);
                }
            }
            else
            {
                Dispose( );
                Close( );
            }
        }

        private void toolStripButton1_Click(object sender, EventArgs e)
        {

        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {
            isSaved = "N";
        }

        private void saveFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            isSaved = "Y";
        }

 
    }
}
