﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace viacode
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main(string[] args )
        {
            if(args.Length != 0)
            {
                foreach(string str in args)
                {
                    MessageBox.Show(str);
                }
                
            }
            Application.EnableVisualStyles( );
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new ViaCode( ));
        }
    }
}
