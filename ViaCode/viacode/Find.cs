using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace viacode
{
    public partial class Find : Form
    {
        public bool isReplace = false;

        private Image find_1 = null;
        private Image find_2 = null;

        public RichTextBox replacetext = null;
        public Button replaceone = null;
        public Button replaceall = null;
        private string imagepath;
        public Find(string path)
        {
            MaximizeBox = false;
            MinimizeBox = false;
            InitializeComponent( );

            imagepath = path +"\\ico\\";
        }
        private void Find_Load(object sender, EventArgs e)
        {
            find_1 = Image.FromFile(imagepath + "find_1.ico");
            find_2 = Image.FromFile(imagepath + "find_2.bmp");

            replacetext = new RichTextBox( );
            replacetext.Size = richTextBox.Size;
            replacetext.Location = new Point(richTextBox.Location.X, richTextBox.Location.Y + 8 + richTextBox.Size.Height);
            replacetext.Visible = false;
            
            replaceone = new Button( );
            replaceone.Size = findbutton.Size;
            replaceone.Location = new Point(replacetext.Location.X, replacetext.Location.Y + 4 + replaceone.Size.Height);
            replaceone.Text = "替换下一个";
            replaceone.Visible = false;

            replaceall = new Button( );
            replaceall.Size = findbutton.Size;
            replaceall.Location = new Point(replacetext.Location.X + (findallbutton.Location.X - findbutton.Location.X), replacetext.Location.Y + 4 + replaceall.Size.Height);
            replaceall.Text = "替换所有";
            replaceall.Visible = false;

            this.Controls.Add(replacetext);
            this.Controls.Add(replaceone);
            this.Controls.Add(replaceall);

            selectBox.SelectedIndex = 0;
            replacebutton.Image = find_1;
        }
 
        private void RefreshFrom()
        {
            int localsize;
            if (isReplace)
            {
                replacebutton.Image = find_2;
                replacetext.Visible = true;
                replaceall.Visible = true;
                replaceone.Visible = true;
                localsize = 50;
            }
            else
            {
                replacetext.Visible = false;
                replaceall.Visible = false;
                replaceone.Visible = false;
                
                replacebutton.Image = find_1;
                localsize = -50;
            }

            this.Size = new Size(Size.Width, Size.Height + localsize);
            selectBox.Location = new Point(selectBox.Location.X, selectBox.Location.Y + localsize);
            findallbutton.Location = new Point(findallbutton.Location.X, findallbutton.Location.Y + localsize);
            findbutton.Location = new Point(findbutton.Location.X, findbutton.Location.Y + localsize);
        }

        private void replacebutton_Click(object sender, EventArgs e)
        {
            isReplace = !isReplace;
            RefreshFrom( );
        }

        private void selectBox_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
