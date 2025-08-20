#include "Mudbus.h"

EthernetServer MbServer(MB_PORT);

static inline int clampRegWindow(int start, int words)
{
  if (start < 0)
    start = 0;
  if (words < 0)
    words = 0;
  if (start >= MB_N_R)
    return 0;
  if (start + words > MB_N_R)
    words = MB_N_R - start;
  return words;
}
static inline int clampCoilWindow(int start, int coils)
{
  if (start < 0)
    start = 0;
  if (coils < 0)
    coils = 0;
  if (start >= MB_N_C)
    return 0;
  if (start + coils > MB_N_C)
    coils = MB_N_C - start;
  return coils;
}

Mudbus::Mudbus() {}

void Mudbus::Run()
{
  Runs = 1 + Runs * (Runs < 999);

  EthernetClient client = MbServer.available();
  if (!client)
    return; // no client at all
  if (!client.connected())
  {
    client.stop();
    return;
  }
  if (!client.available())
    return; // nothing to read right now

  Reads = 1 + Reads * (Reads < 999);

  int i = 0;
  while (client.available() && i < (int)sizeof(ByteArray))
  {
    ByteArray[i++] = client.read();
  }
  if (i < 12)
  {
    client.stop();
    return;
  } // not enough for MBAP+minimal PDU

  SetFC(ByteArray[7]);

  if (!Active)
  {
    Active = true;
    PreviousActivityTime = millis();
  }
  if (millis() - PreviousActivityTime > 60000)
    Active = false;

  int Start, WordLen, ByteLen, CoilLen, MsgLen;

  if (FC == MB_FC_READ_COILS)
  {
    Start = word(ByteArray[8], ByteArray[9]);
    CoilLen = clampCoilWindow(Start, word(ByteArray[10], ByteArray[11]));
    ByteLen = (CoilLen + 7) / 8;

    ByteArray[5] = ByteLen + 3;
    ByteArray[8] = ByteLen;
    memset(&ByteArray[9], 0, ByteLen);

    for (int b = 0; b < ByteLen; b++)
    {
      for (int j = 0; j < 8; j++)
      {
        int idx = Start + b * 8 + j;
        if (idx < Start + CoilLen)
          bitWrite(ByteArray[9 + b], j, C[idx]);
      }
    }
    MsgLen = 6 + ByteArray[5];
    client.write(ByteArray, MsgLen);
    Writes = 1 + Writes * (Writes < 999);
    FC = MB_FC_NONE;
    return;
  }

  if (FC == MB_FC_READ_REGISTERS)
  {
    Start = word(ByteArray[8], ByteArray[9]);
    WordLen = clampRegWindow(Start, word(ByteArray[10], ByteArray[11]));
    ByteLen = WordLen * 2;

    ByteArray[5] = ByteLen + 3;
    ByteArray[8] = ByteLen;

    for (int k = 0; k < WordLen; k++)
    {
      int idx = Start + k;
      ByteArray[9 + k * 2] = highByte(R[idx]);
      ByteArray[10 + k * 2] = lowByte(R[idx]);
    }
    MsgLen = 6 + ByteArray[5];
    client.write(ByteArray, MsgLen);
    Writes = 1 + Writes * (Writes < 999);
    FC = MB_FC_NONE;
    return;
  }

  if (FC == MB_FC_WRITE_COIL)
  {
    Start = word(ByteArray[8], ByteArray[9]);
    if (Start < MB_N_C)
      C[Start] = word(ByteArray[10], ByteArray[11]) > 0;
    ByteArray[5] = 6;
    MsgLen = 12;
    client.write(ByteArray, MsgLen);
    Writes = 1 + Writes * (Writes < 999);
    FC = MB_FC_NONE;
    return;
  }

  if (FC == MB_FC_WRITE_REGISTER)
  {
    Start = word(ByteArray[8], ByteArray[9]);
    if (Start < MB_N_R)
      R[Start] = word(ByteArray[10], ByteArray[11]);
    ByteArray[5] = 6;
    MsgLen = 12;
    client.write(ByteArray, MsgLen);
    Writes = 1 + Writes * (Writes < 999);
    FC = MB_FC_NONE;
    return;
  }

  if (FC == MB_FC_WRITE_MULTIPLE_COILS)
  {
    Start = word(ByteArray[8], ByteArray[9]);
    CoilLen = clampCoilWindow(Start, word(ByteArray[10], ByteArray[11]));
    int ByteCount = ByteArray[12];
    for (int b = 0; b < ByteCount; b++)
    {
      for (int j = 0; j < 8; j++)
      {
        int idx = Start + b * 8 + j;
        if (idx < Start + CoilLen)
          C[idx] = bitRead(ByteArray[13 + b], j);
      }
    }
    ByteArray[5] = 6;
    MsgLen = 12;
    client.write(ByteArray, MsgLen);
    Writes = 1 + Writes * (Writes < 999);
    FC = MB_FC_NONE;
    return;
  }

  if (FC == MB_FC_WRITE_MULTIPLE_REGISTERS)
  {
    Start = word(ByteArray[8], ByteArray[9]);
    WordLen = clampRegWindow(Start, word(ByteArray[10], ByteArray[11]));
    for (int k = 0; k < WordLen; k++)
    {
      int base = 13 + k * 2;
      if (base + 1 < i)
      {
        int idx = Start + k;
        R[idx] = word(ByteArray[base], ByteArray[base + 1]);
      }
    }
    ByteArray[5] = 6;
    MsgLen = 12;
    client.write(ByteArray, MsgLen);
    Writes = 1 + Writes * (Writes < 999);
    FC = MB_FC_NONE;
    return;
  }
}

void Mudbus::SetFC(int fc)
{
  if (fc == 1)
    FC = MB_FC_READ_COILS;
  if (fc == 3)
    FC = MB_FC_READ_REGISTERS;
  if (fc == 5)
    FC = MB_FC_WRITE_COIL;
  if (fc == 6)
    FC = MB_FC_WRITE_REGISTER;
  if (fc == 15)
    FC = MB_FC_WRITE_MULTIPLE_COILS;
  if (fc == 16)
    FC = MB_FC_WRITE_MULTIPLE_REGISTERS;
}