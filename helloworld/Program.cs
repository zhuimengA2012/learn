using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace helloworld
{
    class Program
    {
        static void Main(string[] args)
        {
            int[] num = new int[] { 1, 2, 3, 4, 5 };
            Console.WriteLine("hello world");
            String name = Console.ReadLine();
            Console.WriteLine("{0},{0}",name,name);
            Console.ReadKey();
        }
    }
}
