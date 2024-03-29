﻿using System;
using System.IO.Ports;
using System.Threading;
using System.Text;
using System.Text.RegularExpressions;

namespace ConsoleApplication1
{
    public class PortComm
    {
        static bool _continueRead;
        static bool _continueFlood;
        //static bool _continueSapinDeNoel;
        static bool _continue;
        static SerialPort _serialPort;

        static void Main(string[] args)
        {
            ConsoleKeyInfo cki;
            byte[] str = { 0xAB, 0, 0x64, 0x64, 0x64, 0x64, 0, 0xCD };
            Thread readThread = new Thread(Read);
            Thread floodThread = new Thread(Flood);
            //Thread sapinDeNoelThread = new Thread(SapinDeNoel);
            _serialPort = new SerialPort("COM5", 38400); // Remettre COM1 pour la carte géné
            ASCIIEncoding ascii = new ASCIIEncoding();

            _serialPort.ReadTimeout = 500;
            _serialPort.WriteTimeout = 500;

            _serialPort.Open();
            _continue = true;
            _continueRead = true;
            _continueFlood = false;
            //_continueSapinDeNoel = false;
            readThread.Start();
            floodThread.Start();
            //sapinDeNoelThread.Start();

            Console.WriteLine("Pour obtenir de l'aide, appuyez sur F1");

            while (_continue)
            {
                cki = Console.ReadKey(true);

                if (cki.Key == ConsoleKey.Escape)
                {
                    _continue = false;
                }
                else if (cki.Key == ConsoleKey.Spacebar)  // Mise en pause de l'affichage
                {
                    _continueRead ^= true;
                    Console.WriteLine("Affichage : {0}", _continueRead);
                }
                else if (cki.KeyChar == 'f') // Flood le port série avec des commandes de test. 
                {
                    _continueFlood ^= true;
                    Console.WriteLine("Flood : {0}", _continueFlood);
                }
                else if (cki.Key == ConsoleKey.F1) // Affichage de l'aide
                {
                    Console.WriteLine();
                    Console.WriteLine("###############################################################################");
                    Console.WriteLine("Appuyez sur une touche pour envoyer une commande à la carte géné via son UART1.");
                    Console.WriteLine("La commande envoyée est le code ASCII de la touche appuyée.");
                    Console.WriteLine();
                    Console.WriteLine("Cas particuliers:");
                    Console.WriteLine("Echap: quitte le programme");
                    Console.WriteLine("Barre espace: met en pause l'affichage des caractères reçus");
                    Console.WriteLine("F3: flood la carte avec des commandes \"t\"");
                    Console.WriteLine("un chiffre ou \"/\" ou \"*\": communication avec le clavier (détails: F2)");
                    Console.WriteLine("F4: envoie automatiquement des commandes pour obtenir le sapin de noel");
                    Console.WriteLine("###############################################################################");
                }
                else if (cki.Key == ConsoleKey.F2) // Affichage de l'aide
                {
                    Console.WriteLine();
                    Console.WriteLine("###############################################################################");
                    Console.WriteLine("Detail des touches:");
                    Console.WriteLine("0: UART_CDE_SKIN_INI");
                    Console.WriteLine("1: UART_CDE_SKIN_SEL");
                    Console.WriteLine("2: UART_CDE_SKIN_CLI");
                    Console.WriteLine("3: UART_CDE_HR_INI");
                    Console.WriteLine("4: UART_CDE_HR_SEL");
                    Console.WriteLine("5: UART_CDE_HR_CLI");
                    Console.WriteLine("6: UART_CDE_SR_INI");
                    Console.WriteLine("7: UART_CDE_SR_SEL");
                    Console.WriteLine("8: UART_CDE_SR_CLI");
                    Console.WriteLine("9: UART_CDE_FCT_INI");
                    Console.WriteLine("/: UART_CDE_FCT_SEL");
                    Console.WriteLine("*: UART_CDE_FCT_CLI");
                    Console.WriteLine("Cette commande attend qu'on entre la valeur en hexa de l'octet");
                    Console.WriteLine("que l'on desire envoyer.");
                    Console.WriteLine("Exemple: 5 puis 5F envoie 01011111 en parametre de la commande UART_CDE_HR_CLI");
                    Console.WriteLine("###############################################################################");
                }
                else if (cki.Key == ConsoleKey.F3) // Flood le port série avec des commandes de test. 
                {
                    _continueFlood ^= true;
                    Console.WriteLine("Flood : {0}", _continueFlood);
                }
                /*
                else if ((cki.KeyChar >= '0' && cki.KeyChar <= '9') || cki.KeyChar == '/' || cki.KeyChar == '*') // Envoie d'une commande destiné au clavier
                {
                    str[1] = Convert.ToByte(cki.KeyChar);

                    Console.WriteLine("Commande clavier: {0}", cki.KeyChar);
                    Console.WriteLine("Entrez l'octet à envoyer en hexa:");

                    Char[] octet = { '0', '0' };
                    cki = Console.ReadKey();
                    if ((cki.KeyChar >= '0' && cki.KeyChar <= '9') || (cki.KeyChar >= 'a' && cki.KeyChar <= 'f') || (cki.KeyChar >= 'A' && cki.KeyChar <= 'F'))
                    {
                        octet[0] = cki.KeyChar;
                    }

                    cki = Console.ReadKey();
                    if ((cki.KeyChar >= '0' && cki.KeyChar <= '9') || (cki.KeyChar >= 'a' && cki.KeyChar <= 'f') || (cki.KeyChar >= 'A' && cki.KeyChar <= 'F'))
                    {
                        octet[1] = cki.KeyChar;
                    }
                    str[2] = Convert.ToByte(new String(octet), 16);
                    str[6] = (byte)(str[1] ^ str[2] ^ str[3]);
                    Console.WriteLine();
                    _serialPort.Write(str, 0, 8);
                    str[2] = 0x64;
                }
                else if (cki.Key == ConsoleKey.F4) // Affichage de l'aide
                {
                    _continueSapinDeNoel ^= true;
                    Console.WriteLine();
                    Console.WriteLine("Sapin de noel : {0}", _continueSapinDeNoel);
                }
                */
                else
                {
                    str[1] = Convert.ToByte(cki.KeyChar);
                    str[6] = str[1];
                    //Console.WriteLine(ascii.GetChars(str, 0, 8));
                    _serialPort.Write(str, 0, 8);
                }
            }
        }

        public static void Read()
        {
            String phrase = ""; // Une ligne d'affichage

            while (_continue)
            {
                try
                {
                    string message = _serialPort.ReadExisting();
                    string[] splitPhrase; // Si la chaine contient la fin d'une ligne, on split en deux morceaux
                    
                    if (message.Contains("" + Convert.ToChar(4)))
                    {
                        Console.Clear();
                    }

                    if (message.Contains("\n")) // au moins une ligne de finit
                    {
                        splitPhrase = message.Split('\n');
                        for(int i = 0; i < splitPhrase.Length; i+=2)
                        {
                            phrase += splitPhrase[i];
                            writePhrase(phrase);
                            if (i < splitPhrase.Length-1)
                                phrase = splitPhrase[i + 1];
                            else
                                phrase = "";
                        }
                    }else
                        phrase += message; // Si la phrase n'est pas finit, on ajoute le morceau de message recu au buffer
                }
                catch (TimeoutException) { }
            }
        }

        public static void writePhrase(String phrase)
        {
            if (_continueRead)
            {
                if (phrase.Contains("--") || !phrase.Contains("0x")) // Code pour dire que la phrase courante n'a pas d'hexa a traduire
                {
                    Console.Write(phrase + '\n');
                }
                else // sinon il faut traduire l'hexa (et l'afficher)
                {
                    String hexa = phrase.Substring(phrase.IndexOf("0x") + 2, 8);
                    Console.Write(phrase + " (" + Convert.ToInt32(hexa, 16) + ")\n");
                    hexa = "";
                }
            }
        }

        public static void Flood()
        {
            byte[] str = { 0xAB, 0x74, 0x64, 0x64, 0x64, 0x64, 0x74, 0xCD };
            while (_continue)
            {
                if (_continueFlood)
                {
                    _serialPort.Write(str, 0, 8);
                }
                Thread.Sleep(300);
            }
        }

        /*
        public static void SapinDeNoel()
        {
            byte[] str = { 0xAB, 0x0, 0x0, 0x64, 0x64, 0x64, 0x0, 0xCD };
            Char[] command = { '0', '1', '3', '4', '6', '7', '9', '/' };
            Char[] octet = { '0', 'F' };
            int i = -1;
            while (_continue)
            {
                if (_continueSapinDeNoel)
                {
                    // Incrementation de la commande
                    if (i++ >= 7) i = 0;
                    str[1] = Convert.ToByte(command[i]);

                    str[2] = Convert.ToByte(new String(octet), 16);
                    str[6] = (byte)(str[1] ^ str[2] ^ str[3]);
                    Console.WriteLine("{0}{1}{2}", command[i], octet[0], octet[1]);
                    _serialPort.Write(str, 0, 8);
                }
                else
                {
                    i = -1;
                }
                Thread.Sleep(500);
            }
        }*/
    }
}
