using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using ScintillaNET;

namespace viacode
{
    class file
    {
        private bool _issave;
        private string _name;
        public TreeNode fileinfo;
        public file(Scintilla _text)
        {
            filetext = _text;
        }
        public Scintilla filetext;
        
        public bool Issave
        {
            set
            {
                _issave = value;
            }
            get
            {
                return _issave;
            }
        }

        public TabPage Parent
        {
            set
            {
                filetext.Parent = value;
            }
            get
            {
                return (TabPage)filetext.Parent;
            }
        }
         
        public string Name
        {
            set
            {
                _name = value;
            }
            get
            {
                return _name;
            }
        }
    }
}
