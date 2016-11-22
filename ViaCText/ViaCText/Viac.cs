using System;
using System.Collections.Generic;
using System.Collections;
using System.Drawing;
using System.Linq;
using DMSkin;
using System.Windows.Forms;
using DMSkin.Metro.Controls;
using DMSkin.Controls;
using System.Xml;
using System.Xml.Linq;

namespace ViaCText
{
    public partial class ViaC : Form
    {
        private int childFormNumber = 0;
        /***********************当前窗口设定*************************/
        //文件路径
        private string path;
        //编辑器的名字
        private const string editorname = "ViaC编译器";
        //当前活动的文件
        private RichTextBox nowrich;
        //默认文件名字
        public string defaultname;
        //默认创建的个数
        int defaultnum ;
        Dictionary<int,bool> openrich;
        //默认格式
        /******************工具栏设置********************/
        //当前编码格式
        private string textstyle = "acsii";
        //当前字体
        private Font nowfont;
        //当前字体颜色
        private Color fontcolor = Color.Black;
        int config = 001;
        //当前所有的rich
        List<RichTextBox> richs = new List<RichTextBox>( );
        private string skin = "black.ssk";
        private const string skinpath = "C:\\Users\\Away\\Documents\\ViaC\\ViaCText\\ViaCText\\bin\\Debug\\Skins\\";
        public ViaC()
        {
            InitializeComponent( );
        }
        private void Source()
        {
            XmlDocument config = new XmlDocument( );
            config.Load("C:\\Users\\Away\\Documents\\ViaC\\ViaCText\\ViaCText\\bin\\Debug\\viac.config");
            XmlNode root = config.SelectSingleNode("configuration");
            XmlNodeList nodelist = root.ChildNodes;
            XmlElement configskin = (XmlElement)nodelist[0];
            int size = int.Parse(configskin.GetAttribute("skin"));
            Set(size);
            XmlElement configfont = (XmlElement)nodelist[1];
            string fontname = configfont.GetAttribute("font");
            float fontsize = float.Parse(configfont.GetAttribute("size"));
            nowfont = new Font(fontname, fontsize);
            string configcolor = configfont.GetAttribute("color");
            fontcolor = Color.FromName(configcolor);
            defaultname = ((XmlElement)nodelist[2]).GetAttribute("name");
        }
        private void Set(int num)
        {
            config = num;
            int [] res = new int[3];
            res.Initialize();
            int i = 2;
            while(i >= 0)
            {
                res[i] = num % 10;
                num /= 10;
                --i;
            }
           if(res[0] == 1)
            {
                skin = "sky.ssk";
                skyToolStripMenuItem.Checked = true;
                blackToolStripMenuItem.Checked = false;
                grayToolStripMenuItem.Checked = false;
            }
           else if(res[1] == 1)
            {
                skin = "black.ssk";
                skyToolStripMenuItem.Checked = false;
                blackToolStripMenuItem.Checked = true;
                grayToolStripMenuItem.Checked = false;
            }
           else
            {
                skin = "blue.ssk";
                skyToolStripMenuItem.Checked = false;
                blackToolStripMenuItem.Checked = false;
                grayToolStripMenuItem.Checked = true;
            }
        }
        private void ViaC_Load(object sender, EventArgs e)
        {
            Source( );
            skinEngine1.SkinFile = "C:\\Users\\Away\\Documents\\ViaC\\ViaCText\\ViaCText\\bin\\Debug\\Skins\\" + skin;
            tab.Location = new Point(27, 55);
            this.Controls.Add(this.tab);
            tab.Visible = false;
            tab.Dock = DockStyle.Fill;
            tab.Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top | AnchorStyles.Bottom;
            Text = editorname;
            path = null;
            nowrich = null;
            defaultnum = 1;
            openrich = new Dictionary<int,bool>();
            panel1.Visible = false;
            linenumbox.Font = nowfont;
            tab.DoubleClick += new EventHandler(TabPageDoubleCilck);     
        }
        private void ShowNewForm(object sender, EventArgs e)
        {
            string name = defaultname + defaultnum +"." + defaultname;
            ++defaultnum;
            RichTextBox text = CreateWindow(name);
        }

        private void OpenFile(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog( );
            openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            openFileDialog.Filter = "ViaC文件(*.viac)|*viac|头文件(*.h)|*.h|文本文件(*.txt)|*.txt|所有文件(*.*)|*.*";
            if (openFileDialog.ShowDialog(this) == DialogResult.OK)
            {
               path = openFileDialog.FileName;
                string filename = path.Substring(path.LastIndexOf('\\') + 1);
                RichTextBox text = CreateWindow(filename);
                if (textstyle.Equals("acsii"))
                    text.LoadFile(path, RichTextBoxStreamType.PlainText);
                else
                    text.LoadFile(path, RichTextBoxStreamType.UnicodePlainText);
                if (nowfont != null)
                    text.Font = nowfont;
                Text = editorname + path;
            }
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog( );
            saveFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            saveFileDialog.Filter = "ViaC文件(*.viac)|*.viac|头文件(*.h)|*.h|文本文件(*.txt)|*.txt|所有文件(*.*)|*.*";
            if (saveFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                string FileName = saveFileDialog.FileName;
                path = FileName;
                if (textstyle.Equals("acsii"))
                    nowrich.SaveFile(path , RichTextBoxStreamType.PlainText);
                else
                    nowrich.SaveFile(path , RichTextBoxStreamType.UnicodePlainText);
                string str = path.Substring(path.LastIndexOf("\\") + 1);
                tab.SelectedTab.Text = str;
                openrich.Remove((int)nowrich.Tag);
            }
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (path != null)
            {
                if (textstyle.Equals("acsii"))
                    nowrich.SaveFile(path, RichTextBoxStreamType.PlainText);
                else
                    nowrich.SaveFile(path, RichTextBoxStreamType.UnicodePlainText);
                string str = tab.SelectedTab.Text;
                tab.SelectedTab.Text = str.Substring(0, str.IndexOf('*'));
                openrich.Remove((int)nowrich.Tag);
            }

            else
            {
                SaveAsToolStripMenuItem_Click(sender, e);
            }
        }

        private void ExitToolsStripMenuItem_Click(object sender, EventArgs e)
        {
          
            if (openrich.Count != 0)
            {
                DialogResult  res =   MessageBox.Show("是否保存修改？", "ViaC编译器提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if(res == DialogResult.Yes)
                {
                    saveToolStripMenuItem_Click(sender, e);
                }
               
            }
            else
            {
                this.Close( );
            }
        }

        private void CutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            nowrich.Cut( );
        }

        private void CopyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            nowrich.Copy( );
        }

        private void PasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            nowrich.Paste( );
        }
        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            nowrich.Undo( );
        }

        private void redoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            nowrich.Redo( );
        }
        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            nowrich.SelectAll( );
        }
        private void ToolBarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            toolStrip.Visible = toolBarToolStripMenuItem.Checked;
        }

        private void StatusBarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            statusStrip.Visible = statusBarToolStripMenuItem.Checked;
        }

        private void CascadeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.Cascade);
        }

        private void TileVerticalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.TileVertical);
        }

        private void TileHorizontalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.TileHorizontal);
        }

        private void ArrangeIconsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.ArrangeIcons);
        }

        private void CloseAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (Form childForm in MdiChildren)
            {
                childForm.Close( );
            }
        }

        private void 字体设置ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(fontDialog1.ShowDialog()  == DialogResult.OK)
            {
                nowfont = fontDialog1.Font;
               
                foreach (RichTextBox rich in richs)
                {
                    linenumbox.Font = nowfont;
                    rich.Font = nowfont;
                    rich.ForeColor = fontcolor;
                }
                SaveConfig(config);
            }
          
        }

        private void 字体颜色ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(colorDialog1.ShowDialog() == DialogResult.OK)
            {
                fontcolor = colorDialog1.Color;
                foreach (RichTextBox rich in richs)
                {
                    rich.ForeColor = fontcolor;
                    rich.Font = nowfont;
                }
                SaveConfig(config);
            }
        }
        private RichTextBox CreateWindow(string name)
        {
            TabPage source = new TabPage(name);
            source.AutoScroll = true;
            tab.Controls.Add(source);
            RichTextBox text = new RichTextBox( );
            text.Size = source.Size;
            text.Dock = DockStyle.Fill;
            text.Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top | AnchorStyles.Bottom;
            text.AcceptsTab = true;
            text.TextChanged  += new System.EventHandler(this.richTextBox_TextChanged);
            text.ScrollBars = RichTextBoxScrollBars.Both;
            text.Tag = richs.Count;
            text.Font = nowfont;
            text.ForeColor = fontcolor;
            text.MouseClick += new MouseEventHandler(TextCilck);
            text.VScroll += new EventHandler(textVScroll);
            source.Controls.Add(text);
            source.Tag = 0;
           
            tab.Visible = true;
            text.Visible = true;
            source.Show( );
            panel1.Visible = true;
            nowrich = text;
            tab.SelectedTab = source;
            richs.Add(text);
            return text;
        }

        private void textVScroll(object sender, EventArgs e)
        {
            int d = nowrich.GetPositionFromCharIndex(0).Y %
                    (nowrich.Font.Height + 1);
            linenumbox.Location = new Point(0, d);
            PaintLine( );
        }

        private void TextCilck(object sender, EventArgs e)
        {
            RichTextBox rich = (RichTextBox)sender;
            nowrich = rich;
            GetLine( );
           
        }
        private void PaintLine()
        {
            int num = nowrich.Lines.Length;
            if(num != (int)tab.SelectedTab.Tag)
            {
                tab.SelectedTab.Tag  = num;
                linenumbox.Text = "";
                for (int i = 1; i <= num; i++)
                {
                    linenumbox.Text += i + 1 + "\n";
                }
            }
            

        }

        private void TabPageDoubleCilck(object sender, EventArgs e)
        {
            int index = tab.SelectedIndex;
            TabPage page = tab.TabPages[index];
            if (openrich.ContainsKey((int)nowrich.Tag))
            {
                DialogResult res = MessageBox.Show("是否保存修改？", "ViaC编译器提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if (res == DialogResult.Yes)
                {
                    saveToolStripMenuItem_Click(sender, e);
                    
                }
                openrich.Remove((int)nowrich.Tag); // 不管是否保存都要删除
            }
            if (tab.TabCount >0)
            {
                tab.TabPages.RemoveAt(index);
            }
            if(tab.TabCount == 0)
            {
                tab.Visible = false;
            }
            
        }

        private void richTextBox_TextChanged(object sender, EventArgs e)
        {
            RichTextBox rich = (RichTextBox)sender;
            if (openrich.ContainsKey((int)nowrich.Tag) == false)
            {
                openrich.Add((int)nowrich.Tag, false);
                tab.SelectedTab.Text += "*";
            }
          
            PaintLine();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
           
        }

        private void ViaC_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (openrich.Count != 0)
            {
                DialogResult res = MessageBox.Show("是否保存修改？", "ViaC编译器提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if (res == DialogResult.Yes)
                {
                    saveToolStripMenuItem_Click(sender, e);
                }
            }
            SaveConfig(config);
        }



        private void skyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            skin = "sky.ssk";
            skyToolStripMenuItem.Checked = true;
            blackToolStripMenuItem.Checked = false;
            grayToolStripMenuItem.Checked = false;
            skinEngine1.SkinFile = skinpath + skin;
            config = 100;
            SaveConfig(config);
        }

        private void blackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            skin = "black.ssk";
            skyToolStripMenuItem.Checked = false;
            blackToolStripMenuItem.Checked = true;
            grayToolStripMenuItem.Checked = false;
            skinEngine1.SkinFile = skinpath + skin;
            config = 010;
            SaveConfig(config);
        }

        private void grayToolStripMenuItem_Click(object sender, EventArgs e)
        {
            skin = "blue.ssk";
            skyToolStripMenuItem.Checked = false;
            blackToolStripMenuItem.Checked = false;
            grayToolStripMenuItem.Checked = true;
            skinEngine1.SkinFile = skinpath + skin;
            config = 001;
            SaveConfig(config);
        }
        private void SaveConfig(int num)
        {
            var doc = new XDocument(
                     new XElement("configuration", new XElement("style", new XAttribute("skin", num)),
                       new XElement("font", new XAttribute("font", nowfont.Name), new XAttribute("size", nowfont.Size), new XAttribute("color", fontcolor.Name)), 
                       new XElement("default", new XAttribute("name", defaultname))
                       )
                      );
            doc.Save("viac.config");
        }

        private void cToolStripMenuItem_Click(object sender, EventArgs e)
        {
            defaultname = "c";
            SaveConfig(config);
        }

        private void viacToolStripMenuItem_Click(object sender, EventArgs e)
        {
            defaultname = "viac";
            SaveConfig(config);
        }

        private void hToolStripMenuItem_Click(object sender, EventArgs e)
        {
            defaultname = "h";
            SaveConfig(config);
        }
        private void GetLine()
        {
            int index = nowrich.GetFirstCharIndexOfCurrentLine( );
          
            int line = nowrich.GetLineFromCharIndex(index) + 1;
            int column = nowrich.SelectionStart - index + 1;
            this.toolStripStatusLabel.Text = "状态" + "   " + "行:" + line + "列" + column;
           
        }

     
    }
}