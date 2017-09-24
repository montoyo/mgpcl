/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* This is the test program you may upload to your Arudino
 * to test MGPCL's SerialIO. You'll also have to enable the
 * test first by changing DISABLED_TEST to TEST in IO.cpp
 * and eventually M_TEST_SERIAL_PORT. Also make sure you have
 * sufficient rights to access that port (especially on Linux).
 */

#define BUF_SZ 1024
char g_buf[BUF_SZ];

char readChr()
{
  int ret;
  do {
    ret = Serial.read();
  } while(ret < 0);

  return (char) ret;
}

int readLine()
{
  int i = 0;
  while(i < BUF_SZ) {
    char chr = readChr();
    
    if(chr == '\r') {
      chr = readChr();

      if(chr == '\n') {
        g_buf[i] = 0;
        return i;
      } else {
        g_buf[i++] = '\r';
        g_buf[i++] = chr;
      }
    } else
      g_buf[i++] = chr;
  }

  return -1; //Overflow; shouldn't happen
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  if(readLine() > 0) {
    if(strcmp(g_buf, "What's on this drive?") == 0) {
      Serial.println("Project insight requires... insight");
      Serial.println("So I wrote an algorithm");
    } else if(strcmp(g_buf, "What kind of algorithm? What does it do?") == 0) {
      Serial.println("The answer to your question is fascinating.");
      Serial.println("Unfortunately, you shall be too dead to hear it.");
    }
  }
}
