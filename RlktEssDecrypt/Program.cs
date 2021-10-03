using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Xml;

namespace RlktEssDecrypt
{
    class Program
    {
        static void Main(string[] args)
        {
            byte[] essfile = File.ReadAllBytes(".\\General.ess");

            using(BinaryReader reader = new BinaryReader(new MemoryStream(essfile)))
            {
                reader.ReadUInt16(); // skip 2 bytes
                Dictionary<int, string> dict = new Dictionary<int, string>();

                int nSize = (essfile.Length / 2) - 2;
                char[] array = new char[2048];
                int j = 0;
                for(int i=0;i<nSize-1;i++)
                {
                    char character = (char)reader.ReadUInt16();
                    if(character =='\r')
                    {
                        character = (char)reader.ReadUInt16();
                        if(character == '\n')
                        {
                            string tempstr = new string(array);
                            string[] tempsplitstr = tempstr.Split('\t');

                            int nID = int.Parse(tempsplitstr[0]);
                            string strMessage = tempsplitstr[1].Trim((char)0x0);

                            //Fix chinese?
                            byte[] bytes = Encoding.UTF8.GetBytes(strMessage);
                            string goodDecode = Encoding.UTF8.GetString(bytes);

                            dict.Add(nID, goodDecode);

                            j = 0;
                            array = new char[2048];
                            character = (char)reader.ReadUInt16();//skip the \n
                            i += 2;
                        }
                    }

                    array[j++] = (char)(character ^ (short)16);
                }
                
                XmlWriterSettings xmlWriterSettings = new XmlWriterSettings()
                {
                    Indent = true,
                    IndentChars = "\t",
                    NewLineOnAttributes = true
                };

                XmlWriter xmlWriter = XmlWriter.Create("general.xml", xmlWriterSettings);
                
                xmlWriter.WriteStartDocument();
                xmlWriter.WriteStartElement("messages");


                foreach (KeyValuePair<int,string> keyValuePair in dict)
                {
                    xmlWriter.WriteStartElement("message");
                    xmlWriter.WriteAttributeString("id", keyValuePair.Key.ToString());
                    xmlWriter.WriteCData(keyValuePair.Value);
                    xmlWriter.WriteEndElement();
                    //Console.WriteLine(keyValuePair.Key.ToString() +"="+  keyValuePair.Value);
                }
                xmlWriter.WriteEndDocument();
                xmlWriter.Close();
            }


            Console.WriteLine("ok");
        }
    }
}
