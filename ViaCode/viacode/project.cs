using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace viacode
{
    class Project
    {
        public TreeNode _info = new TreeNode();
        
        public List<string> filelist = new List<string>();
        public List<OpenFile> openlist = new List<OpenFile>( );
        public string dirpath;
        public string projectname;
        public Project(string path, bool isnew)
        {
            dirpath = path.Substring(0, path.LastIndexOf('.'));
            if(!isnew)
            {
                dirpath = dirpath.Substring(0, dirpath.LastIndexOf('\\'));
            }
            projectname = path.Substring(path.LastIndexOf("\\") + 1);
                
            info.Tag = "project";
            info.Name = dirpath + "\\"+ projectname;
            info.Text = projectname.Substring(0, projectname.LastIndexOf('.')); //未包含后缀
        }
        public TreeNode info
        {
            get
            {
                return _info;
            }
            set
            {
                _info = value;
            }
            
        }
        public string Path
        {
            get
            {
                return info.Name;
            }
            set
            {
                info.Name = value;
            }
        }

        public string Name
        {
            get
            {
                return info.Text;
            }
            set
            {
                info.Text = value;
            }
        }

    }
}
