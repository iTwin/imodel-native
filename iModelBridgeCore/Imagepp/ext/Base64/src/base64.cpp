/*
           Encode or decode file as MIME base64 (RFC 1341)
           Port of John Walker's base64 program to library
           by Joel Reed

*/
#include <iostream>

//extern "C" int main() { return 0; } // autoconf help

namespace b64 {
        const int LINELEN=72;                 /* Encoded line length (max 76) */
        typedef unsigned char Byte;           /* Byte type */
        Byte dtable[256];             /* Encode / decode table */
        int linelength = 0;           /* Length of encoded output line */
        char eol[] = "\n";           /* End of line sequence */
        bool errcheck = true;         /* Check decode input for errors ? */

/*  INBUF  --  Fill input buffer with data  */

/*  OCHAR  --  Output an encoded character, inserting line breaks
    where required.     */

        bool ochar(std::ostream& os, int c)
        {
                if (linelength >= LINELEN) {
                        if (os.write(eol, sizeof(eol)/sizeof(char)-1).fail()) {
                                if (errcheck) std::cerr << "ochar failure\n";
                                return false;
                        }
                        linelength = 0;
                }

                if (os.put((char) c).fail()) {
                        if (errcheck) std::cerr << "ochar failure\n";
                        return false;
                }

                linelength++;
                return true;
        }

/*  ENCODE  --  Encode binary file into base64.  */

        bool encode(std::istream& input, std::ostream& output)
        {
                int i; bool hiteof = false;

                /*      Fill dtable with character encodings.  */
                for (i = 0; i < 26; i++) {
                        dtable[i] = 'A' + i;
                        dtable[26 + i] = 'a' + i;
                }
                for (i = 0; i < 10; i++) {
                        dtable[52 + i] = '0' + i;
                }
                dtable[62] = '+';
                dtable[63] = '/';

                while (!hiteof) {
                        Byte igroup[3], ogroup[4];
                        int c=0, n;

                        igroup[0] = igroup[1] = igroup[2] = 0;
                        for (n = 0; n < 3; n++) {
                                c = input.get();
                                if (c == EOF) {
                                        hiteof = true;
                                        break;
                                }
                                igroup[n] = (Byte) c;
                        }
                        if (n > 0) {
                                ogroup[0] = dtable[igroup[0] >> 2];
                                ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
                                ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
                                ogroup[3] = dtable[igroup[2] & 0x3F];

                                /* Replace characters in output stream with "=" pad
                                   characters if fewer than three characters were
                                   read from the end of the input stream. */

                                if (n < 3) {
                                        ogroup[3] = '=';
                                        if (n < 2) {
                                                ogroup[2] = '=';
                                        }
                                }
                                for (i = 0; i < 4; i++) {
                                        if (!ochar(output, ogroup[i]))
                                        {
                                                if (errcheck) std::cerr <<"ERROR writing to output file\n";
                                                return false;
                                        }
                                }
                        }
                }

                if (output.write(eol, sizeof(eol)/sizeof(char)-1).fail())
                {
                        if (errcheck) std::cerr <<"ERROR writing to output file\n";
                        return false;
                }

                return true;
        }

/*  INSIG  --  Return next significant input  */

        int insig(std::istream& input)
        {
                /*CONSTANTCONDITION*/
                while (true) {
                        int c = input.get();
                        if ((c == EOF) || (c > ' ')) {
                                return c;
                        }
                }
                /*NOTREACHED*/
        }

              /*  DECODE  --    Decode base64.  */
        bool decode(std::istream& input, std::ostream& output)
        {
                int i;

                for (i = 0; i < 255; i++) {
                        dtable[i] = 0x80;
                }
                for (i = 'A'; i <= 'Z'; i++) {
                        dtable[i] = 0 + (i - 'A');
                }
                for (i = 'a'; i <= 'z'; i++) {
                        dtable[i] = 26 + (i - 'a');
                }
                for (i = '0'; i <= '9'; i++) {
                        dtable[i] = 52 + (i - '0');
                }
                dtable['+'] = 62;
                dtable['/'] = 63;
                dtable['='] = 0;

                /*CONSTANTCONDITION*/
                while (true) {
                        Byte a[4], b[4], o[3];

                        for (i = 0; i < 4; i++) {
                                int c = insig(input);

                                if (c == EOF) {
                                        if (errcheck && (i > 0)) {
                                                std::cerr <<"Input file incomplete.\n";
                                        }
                                        return false;
                                }
                                if (dtable[c] & 0x80) {
                                        if (errcheck) {
                                                std::cerr << "Illegal character '" << (char)c << "' in input file.\n";
                                        }
                                        /* Ignoring errors: discard invalid character. */
                                        i--;
                                        continue;
                                }
                                a[i] = (Byte) c;
                                b[i] = (Byte) dtable[c];
                        }
                        o[0] = (b[0] << 2) | (b[1] >> 4);
                        o[1] = (b[1] << 4) | (b[2] >> 2);
                        o[2] = (b[2] << 6) | b[3];
                        i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);

                        if (output.write((const char*)o, i).fail()) return false;
                        if (i < 3) return true;
                }
        }
};
