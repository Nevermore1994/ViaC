using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Linq;
using ScintillaNET;


namespace viacode
{
    public partial class ViaCode : Form
    {

        /// <summary>
        /// Todo  把新建文件独立为一个窗口
        /// Todo  
        /// </summary>
        private string ViaCodepath = null;
 
        //默认文件名字
        public string defaultname;
        private int defaultnum = 1;
        //当前编码格式
        private string codestyle = "acsii";
        //当前字体
        private Font nowfont;
        //当前字体颜色
        private Color fontcolor = Color.Black;
        //当前的设置属性
        private int config = 001;
        //当前代码输入框
        private file nowtext;
        //当前页面的最后括号
        private int lastCaretPos = 0;
        //所有的页面
        private List<file> alltexts;
        //皮肤设置
        private string skin = "black.ssk";
        //皮肤路径
        private string skinpath = null;
        //文档属性
        private const string fileclass = "ViaC文件(*.viac)|*.viac|头文件(*.h)|*.h|文本文件(*.txt)|*.txt|所有文件(*.*)|*.*";

        //查找字符串
        private string findstring = null;
        private int findstart = 0;
        private int findend = 0;
        //定义的版本
        private float releaseversion = 0.1078f;
        private float debugversion = 0.1079f;

        //当前使用的版本
        private float version;

        //当前使用的模式
        private bool isproject = false;
        //当前界面偏移量
        //private int local = 0;
        private TreeView projectview = new TreeView( );  //树状图

        //所有打开的项目
        private List<Project> projects;
        //当前打开的项目
        private Project nowproject;
        //当前选择的文件node
        private TreeNode selectnode = null;
        
        public ViaCode()
        {
            InitializeComponent( );
        }

        private void ViaC_Load(object sender, EventArgs e)
        {

            ViaCodepath = Directory.GetCurrentDirectory( );

            version = debugversion;

            Source( );
            skinpath = ViaCodepath + "\\skins\\";
            skinEngine1.SkinFile = skinpath + skin;

            SetSize(isproject);
            tab.Visible = false;
            tab.Click += Tab_Click;
            tab.MouseDoubleClick += Tab_DoubleClick;
            tab.MouseClick += Tab_MouseClick;

            debugBox.Text = "ViaC编译器" + version;
            debugBox.Visible = false;
            debugBox.ReadOnly = true;
            debugBox.TextChanged += DebugBox_TextChanged;

            toolStripStatusLabel.ForeColor = Color.Black;

            alltexts = new List<file>( );

            compiler = new System.Diagnostics.Process( );
            projects = new List<Project>( ); //所有项目
            projectview = new TreeView( ); //树状图

           
        }

        private void Tab_MouseClick(object sender, MouseEventArgs e)
        {
             if(e.Button == MouseButtons.Left)
            {
                if (alltexts.Count > 0)
                {
                    TabPage selectpage = tab.SelectedTab;
                    nowtext = FindFile(selectpage.Name);
                }
            }
            else if(e.Button == MouseButtons.Right)
            {
                
                Point pos = Cursor.Position;
                tab.SelectedTab.ContextMenuStrip.Show(pos);
            }
        }

        /*****************************XML读取函数********************************************/
        private void Source()
        {
            XmlDocument config = new XmlDocument( );
            config.Load(ViaCodepath + "\\viac.config");
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
            version = float.Parse(((XmlElement)nodelist[3]).GetAttribute("version"));
        }

        private void LoadProject(string filepath)
        {
           
            isproject = true;
            if (filepath == null)
                return;
            string filename = filepath.Substring(filepath.LastIndexOf("\\") + 1); //包含了后缀
            XmlDocument loadproject = new XmlDocument( );
            loadproject.Load(filepath);

            if (projectview.Nodes.Count == 0)
                SetSize(isproject);


            Project newpro = new Project(filepath,false);

            nowproject = newpro;
            projectview.Nodes.Add(nowproject.info);
            projectview.SelectedNode = nowproject.info;

            XmlNode root = loadproject.SelectSingleNode("viac" + filename);
            foreach( XmlNode foldnode in root.ChildNodes)
            {
                TreeNode fold = new TreeNode( );
                fold.Tag = "fold";
                fold.Text = foldnode.Name;
                foreach(XmlNode filenode in foldnode.ChildNodes)
                {
                    string tempname = filenode.Name;
                    TreeNode file = new TreeNode( );
                    file.Tag = "file";
                    file.Text =tempname.Substring(tempname.IndexOf("viac")+ 4);
                    file.Name = ((XmlElement)filenode).GetAttribute("path");
                    
                    fold.Nodes.Add(file);
                    nowproject.filelist.Add(file.Name);
                }
                nowproject.info.Nodes.Add(fold);
            }
            nowproject.info.ExpandAll( );

            projectview.Visible = true;
            projects.Add(nowproject);
        }
        /*********************************以下界面函数***********************************/
        private void SetSize(bool isproject)  //重新绘制界面尺寸
        {
            int local = 0;
            if(isproject)
            {
                local = 100;
                
                
                projectview.Size = new Size(local, 575);
                projectview.Location = new Point(5, 55);
                projectview.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Bottom ;
                projectview.NodeMouseClick += Project_NodeMouseClick; ;
                projectview.NodeMouseDoubleClick += Project_NodeMouseDoubleClick;
                this.Controls.Add(projectview);
                projectview.Enabled = true;
                projectview.Visible = false;

            }
            debugBox.Location = new Point(10 + local, 465);
            debugBox.Size = new Size(940 - local, 160);

            tab.Size = new Size(940 - local, 400);
            tab.Location = new Point(10 + local, 55);
        } 

        private void Set(int num)  //设置界面
        {
            config = num;
            int[] res = new int[3];
            res.Initialize( );
            int i = 2;
            while (i >= 0)
            {
                res[i] = num % 10;
                num /= 10;
                --i;
            }
            if (res[0] == 1)
            {
                skin = "sky.ssk";
                skyToolStripMenuItem.Checked = true;
                blackToolStripMenuItem.Checked = false;
                grayToolStripMenuItem.Checked = false;
            }
            else if (res[1] == 1)
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
        /*********************************控件响应函数******************************************/
        #region
        private void LoadFile(string filename)
        {
            try
            {
                if(!File.Exists(filename))
                {
                    MessageBox.Show("文件不存在！", "ViaC Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
                else
                {
                    if (codestyle.Equals("acsii"))
                        TempBox.LoadFile(filename, RichTextBoxStreamType.PlainText);
                    else
                        TempBox.LoadFile(filename, RichTextBoxStreamType.UnicodePlainText);
                }
            }
            catch (IOException io)
            {
                MessageBox.Show("加载文件失败！", "ViaC Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }
        private void Project_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            TreeNode nownode = projectview.SelectedNode;
            if (nownode!= null && nownode.Tag.Equals("file"))
            {
                file resfile = FindFile(nownode.Name);
                if(resfile != null)
                {
                    tab.SelectedTab = resfile.Parent;
                    nowtext = resfile;
                }
                else
                {
                    nowtext = CreateWindow(nownode.Name);
                    nowtext.Parent.Name = nownode.Name;
                    LoadFile(nownode.Name);
            
                    nowtext.filetext.Text = TempBox.Text;
                    nowtext.Issave = true;
                }
               
            }
            if(nownode.Tag.Equals("project"))
            {
               
                projects.Remove(nowproject);
                projectview.Nodes.Remove(nowproject.info);

                if (projects.Count == 0)
                {
                    isproject = false;
                    projectview.Visible = false;
                    SetSize(isproject);
                }
            }
        }

        private void Project_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            TreeNode node = ((TreeView)sender).SelectedNode;
            if(e.Button == MouseButtons.Right)
            {
               
                Point pos = new Point(e.Node.Bounds.X + e.Node.Bounds.Width, e.Node.Bounds.Y + e.Node.Bounds.Height / 2);
                if (node.Tag.Equals("project"))
                {
                    foreach(Project pro in projects)
                    {
                        if(pro.info.Equals(node))
                        {
                            nowproject = pro;
                            break;
                        }
                    }
                    node.ContextMenuStrip = projectMenuStrip;
                }
                else if(node.Tag.Equals("fold"))
                {
                    node.ContextMenuStrip = foldMenuStrip;
                }
                else
                {
                    selectnode = node;
                    node.ContextMenuStrip = fileMenuStrip;
                }

                node.ContextMenuStrip.Show(pos);
            }
            
        }

        private void Project_MouseClick(object sender, MouseEventArgs e)
        {
            
        }

        private void DebugBox_TextChanged(object sender, EventArgs e)
        {
            debugBox.SelectionStart = debugBox.TextLength;
            debugBox.ScrollToCaret( );
        }

        private void Tab_Click(object sender, EventArgs e)
        {
            if (alltexts.Count > 0)
            {
                TabPage selectpage = tab.SelectedTab;
                if(selectpage != null)
                    nowtext = FindFile(selectpage.Name);
            }

        }

        private void CloseFile()
        {
            if (nowtext == null)
                return;
            if (nowtext.Issave == false)
            {
                DialogResult diares = MessageBox.Show("是否保存修改？", "ViaC编译器提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if (diares == DialogResult.Yes)
                {
                    saveToolStripMenuItem_Click(null, null);
                }
                else if(isproject && diares == DialogResult.No)
                {
                    TreeNode parent =  nowtext.fileinfo.Parent;
                    parent.Nodes.Remove(nowtext.fileinfo);
                    SaveProjectConfig( );
                }
                nowtext.Issave = true;
            }

            if (tab.TabCount > 0 && alltexts.Count > 0)
            {
                if(tab.SelectedTab !=null)
                {
                    alltexts.Remove(FindFile(tab.SelectedTab.Name));
                }
                tab.TabPages.Remove(tab.SelectedTab);//remove语句应该放在最后
            }


            if (tab.TabCount == 0)
            {
                tab.Visible = false;
                debugBox.Visible = false;

                alltexts.Clear( );
            }
            if (tab.SelectedTab != null)
                 nowtext = FindFile(tab.SelectedTab.Name);
        }
        
        private void Tab_DoubleClick(object sender, MouseEventArgs e)
        {
            if(e.Button == MouseButtons.Left)
                CloseFile( );
            else
            {
                Point pos = Cursor.Position;
                tab.SelectedTab.ContextMenuStrip.Show(pos);
            }
        }

        #endregion
        //编辑器模板
        private const string templatestr = "#require\"io.viah\" \n" + "int main()\n" + "do\n" + "    return 0;\n" + "end\n" + "\n" + "void _entry()\n" + "do\n    int res;\n" + "    res = main();\n" + "    printf(\"\\n输入任意字符后结束...\\n\");\n" + "    getchar();\n"  + "    exit(res);\n" + "end\n";


        /***************************************以下是Scintilla有关的函数*********************************/
        #region
        private void InitScintilla(Scintilla scintilla)
        {
            scintilla.Tag = 0;


            scintilla.FontQuality = FontQuality.AntiAliased;
            scintilla.Dock = DockStyle.Fill;
            scintilla.Margins.Right = 1;
            scintilla.TextChanged += Myediter_TextChanged;
            scintilla.VScrollBar = true;
            scintilla.StyleResetDefault( );
            scintilla.Styles[Style.Default].Font = "Consolas";
            scintilla.Styles[Style.Default].Size = 16;
            scintilla.StyleClearAll( );

            // Configure the CPP (C#) lexer styles
            scintilla.Styles[Style.Cpp.Default].ForeColor = Color.Silver;
            scintilla.Styles[Style.Cpp.Comment].ForeColor = Color.FromArgb(0, 128, 0); // Green
            scintilla.Styles[Style.Cpp.CommentLine].ForeColor = Color.FromArgb(0, 128, 0); // Green
            scintilla.Styles[Style.Cpp.CommentLineDoc].ForeColor = Color.FromArgb(128, 128, 128); // Gray
            scintilla.Styles[Style.Cpp.Number].ForeColor = Color.Olive;
            scintilla.Styles[Style.Cpp.Word].ForeColor = Color.Blue;
            scintilla.Styles[Style.Cpp.Word2].ForeColor = Color.Blue;
            scintilla.Styles[Style.Cpp.String].ForeColor = Color.FromArgb(163, 21, 21); // Red
            scintilla.Styles[Style.Cpp.Character].ForeColor = Color.FromArgb(163, 21, 21); // Red
            scintilla.Styles[Style.Cpp.Verbatim].ForeColor = Color.FromArgb(163, 21, 21); // Red
            scintilla.Styles[Style.Cpp.StringEol].BackColor = Color.Pink;
            scintilla.Styles[Style.Cpp.Operator].ForeColor = Color.Purple;
            scintilla.Styles[Style.Cpp.Preprocessor].ForeColor = Color.Maroon;

            scintilla.Styles[Style.BraceLight].BackColor = Color.LightGray;
            scintilla.Styles[Style.BraceLight].ForeColor = Color.BlueViolet;
            scintilla.Styles[Style.BraceBad].ForeColor = Color.Red;
            scintilla.ZoomIn( ); //增加 
            scintilla.ZoomOut( ); //减少 
                 

            scintilla.UpdateUI += new EventHandler<UpdateUIEventArgs>(BraceMatching);
            scintilla.CharAdded += Scintilla_CharAdded;
            scintilla.Click += Scintilla_Click;
            scintilla.KeyUp += Scintilla_KeyUp;


            scintilla.Lexer = Lexer.Cpp;
            Console.WriteLine(scintilla.DescribeKeywordSets( ));
            scintilla.SetKeywords(0, "break case continue do end else false for if null return sizeof require");
            scintilla.SetKeywords(1, "char const double enum int string struct void");
        }

        private void Scintilla_KeyUp(object sender, KeyEventArgs e)
        {
            GetLine( );
        }

        private void Scintilla_Click(object sender, EventArgs e)
        {
            GetLine( );
        }

        private void Scintilla_CharAdded(object sender, CharAddedEventArgs e)
        {
            var currentPos = nowtext.filetext.CurrentPosition;
            var wordStartPos = nowtext.filetext.WordStartPosition(currentPos, true);


            var lenEntered = currentPos - wordStartPos;
            if (lenEntered > 0)
            {
                nowtext.filetext.AutoCShow(lenEntered, "break case continue default do end else false for if return require sizeof switch true");
            }
            GetLine( );
            Scintilla text = (Scintilla)sender;
            nowtext = FindFile(tab.SelectedTab.Name);
            if(nowtext.Issave == true)
            {
                nowtext.Issave = false;
                nowtext.Parent.Text += "*";
            }
        }

        private bool IsBrace(int c)
        {
            switch (c)
            {
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '<':
                case '>':
                    return true;
            }

            return false;
        }

        private bool IsBrace(string token)
        {
            if (token.Equals("do") || token.Equals("end"))
            {
                return true;
            }
            return false;
        }

        private void BraceMatching(object sender, UpdateUIEventArgs e)
        {
            if (nowtext == null)
                return;
            var caretPos = nowtext.filetext.CurrentPosition;
            if (lastCaretPos != caretPos)
            {
                lastCaretPos = caretPos;
                var bracePos1 = -1;
                var bracePos2 = -1;

                // Is there a brace to the left or right?
                if (caretPos > 0 && IsBrace(nowtext.filetext.GetCharAt(caretPos - 1)))
                    bracePos1 = (caretPos - 1);
                else if (IsBrace(nowtext.filetext.GetCharAt(caretPos)))
                    bracePos1 = caretPos;

                if (bracePos1 >= 0)
                {

                    bracePos2 = nowtext.filetext.BraceMatch(bracePos1);
                    if (bracePos2 == Scintilla.InvalidPosition)
                    {
                        nowtext.filetext.BraceBadLight(bracePos1);
                        nowtext.filetext.HighlightGuide = 0;
                    }
                    else
                    {
                        nowtext.filetext.BraceHighlight(bracePos1, bracePos2);
                        nowtext.filetext.HighlightGuide = nowtext.filetext.GetColumn(bracePos1);
                    }
                }
                else
                {
                    nowtext.filetext.BraceHighlight(Scintilla.InvalidPosition, Scintilla.InvalidPosition);
                    nowtext.filetext.HighlightGuide = 0;
                }
            }
        }

        private void Myediter_TextChanged(object sender, EventArgs e)
        {
            Scintilla text = (Scintilla)sender;
            var maxLineNumberCharLength = text.Lines.Count.ToString( ).Length;
            if (maxLineNumberCharLength == (int)text.Tag)
                return;
            const int padding = 2;
            text.Margins[0].Width = text.TextWidth(Style.LineNumber, new string('9', maxLineNumberCharLength + 1)) + padding;
            text.Tag = maxLineNumberCharLength;
            nowtext = FindFile(tab.SelectedTab.Name);
            if(nowtext.Issave == true)
            {
                nowtext.Parent.Text += "*";
                nowtext.Issave = false;
            }
            GetLine( );

        }
        #endregion
        /***************************窗口创建函数**************************************************************/
        #region
        private void CreatrProjectWindow(string filepath)
        {
            if(projectview.Nodes.Count == 0)
                SetSize(isproject);

            Project newpro = new Project(filepath,true);
            
            projectview.Nodes.Add(newpro.info);
            projectview.SelectedNode = newpro.info;
            newpro.info.EnsureVisible( );
            
            TreeNode cfile = new TreeNode("源文件");
            cfile.Tag = "fold";
            cfile.Name = newpro.dirpath + "\\" +"源文件";
            Directory.CreateDirectory(cfile.Name);

            TreeNode hfile = new TreeNode("头文件");
            hfile.Tag = "fold";
            hfile.Name = newpro.dirpath + "\\" + "头文件";
            Directory.CreateDirectory(hfile.Name);

            TreeNode ofile = new TreeNode("其他文件");
            ofile.Tag = "fold";
            ofile.Name = newpro.dirpath+ "\\"  + "其他文件";
            Directory.CreateDirectory(ofile.Name);

            projectview.SelectedNode.Nodes.Add(cfile);
            projectview.SelectedNode.Nodes.Add(hfile);
            projectview.SelectedNode.Nodes.Add(ofile);
            projectview.SelectedNode.ExpandAll( );

            projectview.Visible = true;

            nowproject = newpro;
            projects.Add(nowproject);

            SaveProjectConfig( );
        }

        private file CreateWindow(string filename)
        {
            int isnew = filename.LastIndexOf('\\');
            string name = filename.Substring(isnew + 1);

            Scintilla Myediter = new Scintilla( );
            InitScintilla(Myediter);

            nowtext = new file(Myediter);
            nowtext.Issave = true;
            TabPage tabPage = new TabPage( );
            tabPage.Text = name;
            if (isnew == -1)
            {
                tabPage.Name = "null";
                tabPage.Tag = Environment.CurrentDirectory + name;
               
            }
            else
            {
                tabPage.Name = filename;
                tabPage.Tag = filename;
            }
            tabPage.ContextMenuStrip = tabMenuStrip;
            tabPage.MouseClick += TabPage_MouseClick;
            
            tabPage.Controls.Add(nowtext.filetext);
            nowtext.Parent = tabPage;   //需要设置
            
            tabPage.Show( );
            tabPage.AutoScroll = true;

            
            tab.Controls.Add(tabPage);
            tab.SelectedTab = tabPage;
            tab.Visible = true;
            debugBox.Visible = true;

            alltexts.Add(nowtext);

            nowtext.filetext.Text = templatestr;
            return nowtext;
        }

        private void TabPage_MouseClick(object sender, MouseEventArgs e)
        {
            if(e.Button == MouseButtons.Right)
            {
                Point pos = new Point(e.X, e.Y);
                tab.SelectedTab.ContextMenuStrip.Show(pos);
            }
        }
        #endregion
        /************************以下是IDE的功能函数*********************************************************/
        #region
        private void newToolStripButton_Click(object sender, EventArgs e)
        {
            string name = defaultname + defaultnum + "." + defaultname;
            ++defaultnum;

            CreateWindow(name);
        }

        private void 文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string name = defaultname + defaultnum + "." + defaultname;
            ++defaultnum;

            CreateWindow(name);
        }

        private void 项目ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog prodialog = new SaveFileDialog( );
            prodialog.Title = " ViaC项目新建";
            prodialog.Filter = "ViaC项目(*.viacproject)|*.viacproject";
            if(prodialog.ShowDialog() == DialogResult.OK)
            {
                string  proname = prodialog.FileName;
                string directory = proname.Substring(0, proname.LastIndexOf('.') );
                if( !Directory.Exists(directory) )
                {
                    Directory.CreateDirectory(directory);
                }
                
                isproject = true;
                if(projectview.Nodes.Count == 0)
                    SetSize(isproject);
                CreatrProjectWindow(proname);
            }
            prodialog.Dispose( );
        }

        private void OpenFile(object sender, EventArgs e)
        {
            现有文件ToolStripMenuItem1_Click(null, null);
        }

        private void 现有文件ToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog( );
            openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            openFileDialog.Filter = fileclass;
            if (openFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                string path = openFileDialog.FileName;
                nowtext = CreateWindow(path);
                LoadFile(path);

                nowtext.filetext.Text = TempBox.Text;
            }
            openFileDialog.Dispose( );
        }

        private void 现有项目ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog( );
            openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            openFileDialog.Filter = "ViaC项目(*.viacproject)|*.viacproject";
            if (openFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                string path = openFileDialog.FileName;

                LoadProject(path);
            }
            openFileDialog.Dispose( );
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(!isproject && nowtext == null)
            {
                MessageBox.Show("未打开任何的文件！", "ViaC Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            if(isproject)
            {
                SaveProjectConfig( );
            }
            if (nowtext !=null && !nowtext.Parent.Name.Equals("null") )
            {
                TempBox.Text = nowtext.filetext.Text;
                if (codestyle.Equals("acsii"))
                    TempBox.SaveFile(nowtext.Parent.Name, RichTextBoxStreamType.PlainText);
                else
                    TempBox.SaveFile(nowtext.Parent.Name, RichTextBoxStreamType.UnicodePlainText);
                TabPage page = (TabPage)nowtext.Parent;
                string str = page.Text;
                int ischange = str.IndexOf('*');
                if (ischange > -1)
                {
                    page.Text = str.Substring(0, ischange);
                }
                nowtext.Issave = true;
            }
            else if(nowtext != null)
            {
                SaveAsToolStripMenuItem_Click(null, null);
            }
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog( );
            if (isproject)
                saveFileDialog.InitialDirectory = nowproject.dirpath;
            saveFileDialog.Filter = fileclass;
            if (saveFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                string FileName = saveFileDialog.FileName;
                TempBox.Text = nowtext.filetext.Text;
                if (codestyle.Equals("acsii"))
                    TempBox.SaveFile(FileName, RichTextBoxStreamType.PlainText);
                else
                    TempBox.SaveFile(FileName, RichTextBoxStreamType.UnicodePlainText);
                string str = FileName.Substring(FileName.LastIndexOf("\\") + 1);

                nowtext.Parent.Text = str;
                nowtext.Parent.Name = FileName;

                //项目里名字也要改变
                if(isproject)
                {
                    nowtext.fileinfo.Text = str;
                    nowtext.fileinfo.Name = FileName;
                    SaveProjectConfig( );
                }
                nowtext.Issave = true;
            }
            else if(isproject )
            {
                TreeNode parent = nowtext.fileinfo.Parent;
                parent.Nodes.Remove(nowtext.fileinfo);

                SaveProjectConfig( );
            }
            saveFileDialog.Dispose( );
        }

        private void ExitApplication()
        {
            if (!isproject)
            {
                foreach (file page in alltexts)
                {
                    if (page.Issave == false)
                    {
                        DialogResult res = MessageBox.Show("是否保存修改？", "ViaC编译器提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                        if (res == DialogResult.Yes)
                        {
                            saveToolStripMenuItem_Click(null, null);
                        }
                       
                    }
                }
            }
            else
            {
                foreach (Project pro in projects)
                {
                     nowproject = pro;
                     CloseProject( );
                }
            }

            SaveApplicationConfig(config); //保存应用设置
        }

        private void ExitToolsStripMenuItem_Click(object sender, EventArgs e)
        {
            ExitApplication( );
            this.Close( );
        }

        private void ViaCode_FormClosing(object sender, FormClosingEventArgs e)
        {
            ExitApplication( );
        }

        private void ToolBarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
                toolStrip.Visible = toolBarToolStripMenuItem.Checked;
        }

        private void StatusBarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
                statusStrip.Visible = statusBarToolStripMenuItem.Checked;
        }

        private void PasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
            {
                nowtext.filetext.Paste( );
                if (nowtext.Issave == true)
                {
                    TabPage page = (TabPage)nowtext.Parent;
                    nowtext.Issave = false;
                    page.Text += "*";
                }
            }

        }

        private void CopyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
            {
                nowtext.filetext.Copy( );
                if (nowtext.Issave == true)
                {
                    TabPage page = (TabPage)nowtext.Parent;
                    nowtext.Issave = false;
                    page.Text += "*";
                }
            }
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
            {
                nowtext.filetext.Undo( );
                if (nowtext.Issave == true)
                {
                    TabPage page = (TabPage)nowtext.Parent;
                    nowtext.Issave = false;
                    page.Text += "*";
                }
            }

        }

        private void CutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
            {
                nowtext.filetext.Cut( );
                if (nowtext.Issave == true)
                {
                    TabPage page = (TabPage)nowtext.Parent;
                    nowtext.Issave = false;
                    page.Text += "*";
                }
            }

        }

        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
            {
                nowtext.filetext.SelectAll( );
            }
        }

        private void 重复ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
                nowtext.filetext.InsertText(nowtext.filetext.SelectionEnd, "\n" + nowtext.filetext.SelectedText);
        }

        private void 查找ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (nowtext != null)
            {
                TempBox.Text = nowtext.filetext.Text;
                findstart = 0;

                Form findform = new Form( );
                findform.Size = new Size(200, 150);
                findform.Text = "查找";
                findform.MaximizeBox = false;
                findform.MinimizeBox = false;

                TextBox findbox = new TextBox( );
                findbox.Size = new Size(150, 100);
                findbox.Visible = true;
                findbox.Location = new Point(25, 20);
                findbox.TextChanged += Findbox_TextChanged;

                Button find = new Button( );
                find.Size = new Size(80, 20);
                find.Location = new Point(20, 70);
                find.Text = "查找下一个";
                find.MouseClick += Find_MouseClick;
                Button findall = new Button( );
                findall.Size = new Size(80, 20);
                findall.Location = new Point(110, 70);
                findall.Text = "查找全部";
                findall.MouseClick += Findall_MouseClick;

                findform.Controls.Add(find);
                findform.Controls.Add(findall);
                findform.Controls.Add(findbox);
                findform.Show( );
                findform.FormClosed += Findform_FormClosed;
            }

        }

        private void Findform_FormClosed(object sender, FormClosedEventArgs e)
        {
            nowtext.filetext.Text = TempBox.Text;
        }

        private void Findall_MouseClick(object sender, MouseEventArgs e)
        {
            const int NUM = 8;

            // Remove all uses of our indicator
            nowtext.filetext.IndicatorCurrent = NUM;
            nowtext.filetext.IndicatorClearRange(0, nowtext.filetext.TextLength);

            // Update indicator appearance
            nowtext.filetext.Indicators[NUM].Style = IndicatorStyle.StraightBox;
            nowtext.filetext.Indicators[NUM].Under = true;
            nowtext.filetext.Indicators[NUM].ForeColor = Color.Green;
            nowtext.filetext.Indicators[NUM].OutlineAlpha = 50;
            nowtext.filetext.Indicators[NUM].Alpha = 30;

            // Search the document
            nowtext.filetext.TargetStart = 0;
            nowtext.filetext.TargetEnd = nowtext.filetext.TextLength;
            nowtext.filetext.SearchFlags = SearchFlags.None;
            while (nowtext.filetext.SearchInTarget(findstring) != -1)
            {
                // Mark the search results with the current indicator
                nowtext.filetext.IndicatorFillRange(nowtext.filetext.TargetStart, nowtext.filetext.TargetEnd - nowtext.filetext.TargetStart);

                // Search the remainder of the document
                nowtext.filetext.TargetStart = nowtext.filetext.TargetEnd;
                nowtext.filetext.TargetEnd = nowtext.filetext.TextLength;
            }
        }

        private void Findbox_TextChanged(object sender, EventArgs e)
        {
            TextBox text = (TextBox)sender;
            findstring = text.Text;
            findend = nowtext.filetext.TextLength;
        }

        private void Find_MouseClick(object sender, MouseEventArgs e)
        {
            const int NUM = 8;

            // Remove all uses of our indicator
            nowtext.filetext.IndicatorCurrent = NUM;
            nowtext.filetext.IndicatorClearRange(findstart, findend);

            // Update indicator appearance
            nowtext.filetext.Indicators[NUM].Style = IndicatorStyle.StraightBox;
            nowtext.filetext.Indicators[NUM].Under = true;
            nowtext.filetext.Indicators[NUM].ForeColor = Color.Red;
            nowtext.filetext.Indicators[NUM].OutlineAlpha = 50;
            nowtext.filetext.Indicators[NUM].Alpha = 30;

            // Search the document
            nowtext.filetext.TargetStart = findstart;
            nowtext.filetext.TargetEnd = findend;

            nowtext.filetext.SearchFlags = SearchFlags.None;
            if (nowtext.filetext.SearchInTarget(findstring) != -1)
            {
                // Mark the search results with the current indicator
                nowtext.filetext.IndicatorFillRange(nowtext.filetext.TargetStart, nowtext.filetext.TargetEnd - nowtext.filetext.TargetStart);

                // Search the remainder of the document
                findstart = nowtext.filetext.TargetEnd;
                findend = nowtext.filetext.TextLength;
            }

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
            tab.TabPages.Clear( );
        }

        private void 字体设置ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (fontDialog.ShowDialog( ) == DialogResult.OK)
            {
                nowfont = fontDialog.Font;
                foreach (file text in alltexts)
                {
                    text.filetext.Font = nowfont;
                }
                SaveApplicationConfig(config);
            }
        }

        private void 字体颜色ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (colorDialog.ShowDialog( ) == DialogResult.OK)
            {
                fontcolor = colorDialog.Color;
                foreach (file text in alltexts)
                {
                    text.filetext.CaretForeColor = fontcolor;
                }
                SaveApplicationConfig(config);
            }
        }

        private void skyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            skin = "sky.ssk";
            skyToolStripMenuItem.Checked = true;
            blackToolStripMenuItem.Checked = false;
            grayToolStripMenuItem.Checked = false;
            skinEngine1.SkinFile = skinpath + skin;
            config = 100;
            SaveApplicationConfig(config);
        }

        private void blackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            skin = "black.ssk";
            skyToolStripMenuItem.Checked = false;
            blackToolStripMenuItem.Checked = true;
            grayToolStripMenuItem.Checked = false;
            skinEngine1.SkinFile = skinpath + skin;
            config = 010;
            SaveApplicationConfig(config);
        }

        private void grayToolStripMenuItem_Click(object sender, EventArgs e)
        {
            skin = "blue.ssk";
            skyToolStripMenuItem.Checked = false;
            blackToolStripMenuItem.Checked = false;
            grayToolStripMenuItem.Checked = true;
            skinEngine1.SkinFile = skinpath + skin;
            config = 001;
            SaveApplicationConfig(config);
        }

        private void cToolStripMenuItem_Click(object sender, EventArgs e)
        {
            defaultname = "c";
            SaveApplicationConfig(config);
        }

        private void viacToolStripMenuItem_Click(object sender, EventArgs e)
        {
            defaultname = "viac";
            SaveApplicationConfig(config);
        }

        private void hToolStripMenuItem_Click(object sender, EventArgs e)
        {
            defaultname = "h";
            SaveApplicationConfig(config);
        }

        private void 稳定版ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("当前版本:" + releaseversion, "ViaC Help", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            version = releaseversion;
        }

        private void 开发版ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("当前版本:" + debugversion, "ViaC Help", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            version = debugversion;
        }

        private void 编译ToolStripMenuItem_Click(object sender, EventArgs e)
        {

            try
            {
                saveToolStripMenuItem_Click(null, null);
                resname = Compile(nowtext.filetext.Parent.Name);
            }
            catch
            {
                MessageBox.Show("请检查是否有文件名错误", "ViaC Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void 编译并运行ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            编译ToolStripMenuItem_Click(null, null);
            if (generateres == 0)
            {
                DialogResult res = MessageBox.Show("生成失败，是否启动上次的生成", "ViaC Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                if (res == DialogResult.Yes)
                    generateres = 1;
            }
            /****************************启动进程的应用函数******************************************/
            #region
            if (generateres == 1)
            {
                Process[] ps = Process.GetProcesses( );
                foreach (Process item in ps) //如果当前的进程已经在运行，那么应该将其Kill
                {
                    if (item.ProcessName == resname)
                    {
                        item.Kill( );
                    }
                }
                System.Diagnostics.Process resexe = new Process( );
                try
                {
                    resexe.StartInfo.FileName = resname;
                    resexe.StartInfo.UseShellExecute = false;
                    resexe.StartInfo.RedirectStandardInput = false;
                    resexe.StartInfo.RedirectStandardOutput = false;
                    resexe.Start( );
                    //等待当前的程序运行完毕
                    resexe.WaitForExit( );
                }
                catch
                {
                    MessageBox.Show("启动应用失败,可能不存在", "viac警告");
                }
                finally
                {
                    resexe.Close( );
                }

            }
            #endregion

        }

        private void 调试ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("调试");
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutBox about = new AboutBox( );
            about.Enabled = true;
            about.Activate( );

            about.Show( );
        }

        #endregion

        private void GetLine() //光标状态函数
        {

            int line = nowtext.filetext.CurrentLine + 1;
            int row = nowtext.filetext.GetColumn(nowtext.filetext.CurrentPosition);
            this.toolStripStatusLabel.Text = "状态" + "   " + "行:" + line +"      列:"+ row;
        }
    
        /**********************************以下是DLL调用*****************************************************/
        #region
        [DllImport("cdll.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        extern static int Free();
        [DllImport("cdll.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr main(int length, IntPtr[] argv);
        #endregion

        /**********************************以下是关于编译模块****************************************************/
        private static System.Diagnostics.Process compiler = null; //cmd的进程
        private string resname = null; //产生的名字
        private int generateres = 0;  //默认产生失败
        private string viacpath = "cd "; //编译器的路径

        private string  Compile(string path)
        {

            string headfile = null;

            /*******************************编译前的处理，头文件机制***********************************/
            #region 

            try
            {
                StreamReader sr;
                if (codestyle.Equals("acsii"))
                    sr = new StreamReader(path, Encoding.ASCII);
                else
                    sr = new StreamReader(path, Encoding.Unicode);
                string line;

                while ((line = sr.ReadLine()) != null)
                {
                    if (line.Contains("#"))
                    {
                        string head = line.Substring(line.IndexOf("\"") + 1);
                        head = head.Substring(0, head.IndexOf(".viah"));
                        switch(head)
                        {
                            case "io":
                                {
                                    headfile += "-lmsvcrt" + " ";
                                    break;
                                }
                            case "math":
                                {
                                    headfile += "-lmath" + " ";
                                    break;                                    
                                }
                            case "windows":
                                {
                                    headfile += " -lmsvcrt - lgdi32 - luser32 - lkernel32 - lcomdlg32" + " ";
                                    break;
                                }
                        }
                
                    }
                }
                sr.Close( ); //记得释放资源
            }
            catch
            {
                MessageBox.Show("请先保存文件", "viac warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            #endregion
            
            generateres = 0; //返回标志。如果是1的话，那么就是成功了
            
            /**********************如果已存在了进程，则需要将其kill**************************/
            Process[] ps = Process.GetProcesses( );
            foreach (Process item in ps)
            {
                if (item.ProcessName == "viac")
                {
                    item.Kill( );
                }
            }
            /********************得到文件路径**************************************/
            string exeres = path.Substring(0,path.LastIndexOf('.') + 1);
            exeres += "exe";
            string[] argv = new string[5];
            argv[0] = "viac";
            argv[1] = headfile;
            argv[2] = "-o";
            argv[3] = exeres;
            argv[4] = path;


            /************************************以下是输入至CMD进程中的参数*******************************************/
            #region
              string command = argv[0] + " " + argv[1] + " " + argv[2] + " " + argv[3] + " " + argv[4];
              compiler.StartInfo.FileName = "cmd.exe";
              compiler.StartInfo.UseShellExecute = false;    
              compiler.StartInfo.RedirectStandardInput = true;
              compiler.StartInfo.RedirectStandardOutput = true;
              compiler.StartInfo.RedirectStandardError = true;
              compiler.StartInfo.CreateNoWindow = true;
              compiler.Start( );

              viacpath = "cd " + ViaCodepath;
              compiler.StandardInput.WriteLine(viacpath);  //open path

              compiler.StandardInput.WriteLine(command + " &exit");  //必须跟exit，否则会导致命令无法执行
              compiler.StandardInput.AutoFlush = true;  //立即输出

              compiler.WaitForExit( );
              string output = compiler.StandardOutput.ReadToEnd( );
              compiler.Close( ); //释放资源

              if (output.Contains("编译成功"))
              {
                  generateres = 1;  //生成成功
              }
              else
              {
                  output += "编译失败";
              }

              debugBox.Text = output;
            #endregion

            return exeres;
        }
        
        /*************************************以下关于项目模块函数***********************************************/
        #region
        private void CloseProject()
        {
            foreach (TabPage page in tab.TabPages)
            {
                file resfile = FindFile(page.Name);
                if ( resfile !=  null)
                {
                    nowtext = resfile;
                    CloseFile( );
                    tab.TabPages.Remove(page);
                }
            }
            if (tab.SelectedTab != null)
                nowtext = FindFile(tab.SelectedTab.Name);
            if (projects.Count == 0)
            {
                isproject = false;
                projectview.Visible = false;
                SetSize(isproject);
            }

            projectview.Nodes.Remove(nowproject.info);
            SaveProjectConfig( );
        }
        private void 关闭项目ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            projects.Remove(nowproject);
            CloseProject( );
        }

        private void 添加文件夹ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog folddialog = new FolderBrowserDialog( );
            folddialog.SelectedPath = nowproject.dirpath;
            folddialog.ShowNewFolderButton = true;
            if ( folddialog.ShowDialog() == DialogResult.OK)
            {
                string foldpath = folddialog.SelectedPath;
                DirectoryInfo direct = new DirectoryInfo(foldpath);
                if(!direct.Exists)
                {
                    direct.Create( );
                }
                    
                TreeNode newfold = new TreeNode(foldpath.Substring(foldpath.LastIndexOf("\\") + 1));
                newfold.Name = foldpath;
                newfold.Tag = "fold";
                nowproject.info.Nodes.Add(newfold);
            }
            folddialog.Dispose( );
            SaveProjectConfig( );
        }

        private void 删除文件夹ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode nownode = projectview.SelectedNode;
            TreeNode node = nownode.Parent;
            node.Nodes.Remove(nownode);
        }

        private void 现有文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog filedialog = new OpenFileDialog( );
            filedialog.Filter = fileclass;
            if (filedialog.ShowDialog( ) == DialogResult.OK)
            {
                string filepath = filedialog.FileName;

                if(FindFile(filepath) != null)
                {
                    if(MessageBox.Show("该文件已在项目中！", "ViaC Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning) == DialogResult.OK)
                    {
                        
                        return;  //已经在项目中的不能重复添加
                    }
                }
                nowtext = CreateWindow(filepath);
                LoadFile(filepath);

                TreeNode newfile = new TreeNode(filepath.Substring(filepath.LastIndexOf("\\") + 1));
                newfile.Name = filepath;
                newfile.Tag = "file";
                nowtext.fileinfo = newfile;
                projectview.SelectedNode.Nodes.Add(newfile);

                nowproject.filelist.Add(newfile.Name);
                SaveProjectConfig( );
            }
            filedialog.Dispose( );
        }

        private void 新建文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
             string name = defaultname + defaultnum + "." + defaultname;
             ++defaultnum;
            
            
           string path = null;
            if(projectview.SelectedNode.Tag.Equals("fold"))
            {
                path = projectview.SelectedNode.Name;
            }
            if (path == null)
                return;
            nowtext = CreateWindow(name);
            TreeNode newfile = new TreeNode();
            newfile.Name = path + "\\" + name;
            newfile.Tag = "file";
            newfile.Text = name;
            nowtext.fileinfo = newfile;

            projectview.SelectedNode.Nodes.Add(newfile);
            projectview.SelectedNode = newfile;

            nowproject.filelist.Add(newfile.Name);
            SaveProjectConfig( );

        }

        private void 打开文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Project_NodeMouseDoubleClick(null, null);
        }

        private void 关闭文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CloseFile( );
        }

        private void 排除文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode nownode = projectview.SelectedNode;
            if (nownode.Tag.Equals("file"))
            {
                TreeNode node = nownode.Parent;
                node.Nodes.Remove(nownode);
            }
            SaveProjectConfig( );
        }

        private void 关闭ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CloseFile( );
        }

        private void 保存ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveToolStripMenuItem_Click(null, null);
        }

        private void saveToolStripButton_Click(object sender, EventArgs e)
        {
            saveToolStripMenuItem_Click(null, null);
        }

        private void helpToolStripButton_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("https://github.com/as981242002/ViaC");
        }

        private void 删除文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode nownode = projectview.SelectedNode;
            if (nownode.Tag.Equals("file"))
            {
                TreeNode node = nownode.Parent;
                try
                {
                    if (File.Exists(nownode.Name))
                    {
                        File.Delete(nownode.Name);
                    }
                }
                catch
                {
                    MessageBox.Show("文件不存在或是文件正在被使用！", "ViaC Warning" , MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
                node.Nodes.Remove(nownode);
            }
            SaveProjectConfig( );
        }
        #endregion
        /***************************************以下是项目功能函数***************************************************/
        private file FindFile(string filename)
        {
            file resfile = null;
            
            if(alltexts.Count > 0)
            {
                foreach(file nowfile in alltexts)
                {
                    if(nowfile.Parent.Name.Equals(filename))
                    {
                        resfile = nowfile;
                        return resfile;
                    }
                }
            } 
            return resfile;
        }
        /***************************************以下是文件XML保存函数************************************************/
        #region
        private void SaveApplicationConfig(int num)
        {
            var doc = new XDocument(
                     new XElement("configuration", new XElement("style", new XAttribute("skin", num)),
                       new XElement("font", new XAttribute("font", nowfont.Name), new XAttribute("size", nowfont.Size), new XAttribute("color", fontcolor.Name)),
                       new XElement("default", new XAttribute("name", defaultname)),
                        new XElement("version", new XAttribute("version", version))
                       )
                      );
            doc.Save(ViaCodepath + "\\viac.config");
        }

        private void SaveProjectConfig()
        {
            TreeNode nownode = null;
            if (projectview.SelectedNode != null)
            {
                nownode = projectview.SelectedNode;
                while (!nownode.Tag.Equals("project"))
                {
                    nownode = nownode.Parent;
                }

                List<XElement> allxml = new List<XElement>( );
                for(int i=0; i<nownode.Nodes.Count; ++i)
                {
                    allxml.Add(new XElement(nownode.Nodes[i].Text, GetXel(nownode.Nodes[i].Nodes) ));
                }
                XDocument doc = new XDocument(new XElement("viac" + nownode.Text + ".viacproject", allxml));

                doc.Save(nownode.Name);

            }
        }

        private List<XElement> GetXel(TreeNodeCollection nodes)
        {
            List<XElement> resxml = new List<XElement>();
            if(nodes.Count > 0)
            {
                foreach (TreeNode node in nodes)
                {
                    var xtt = new XAttribute("path", node.Name);
                    XElement xml = new XElement("viac" + node.Text, xtt); //XML中不能出现数字标签，以filename为开头，以防出现数字命名的文件
                    resxml.Add(xml);
                }
            }
            return resxml;
        }


        #endregion

      
    }
}
