////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU_Impl_O1.cpp
///  Description: The CPU implementation for the AES using medium optimizations
///               ( MixColumns using LUT and other functions made in same
///                 step, also decryption is done using Equivalent Inverse Cipher )
///  Author:      Chiuta Adrian Marius
///  Created:     12-11-2009
///
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///  http://www.apache.org/licenses/LICENSE-2.0
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "platform.h"
#include "AES_CPU_Impl_O1.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//The following tables were generated using the code:
//
//void AddTable(FILE* fout, const char *tableName, UINT32 v1, UINT32 v2, UINT32 v3, UINT32 v4)
//{
//  fprintf(fout, "/*\n\tFor every value found at index we have:\n"
//                "\t\t- bits 0..7 represent Multiply(index, 0x%02X).\n"
//                "\t\t- bits 8..15 represent Multiply(index, 0x%02X).\n"
//                "\t\t- bits 16..23 represent Multiply(index, 0x%02X).\n"
//                "\t\t- bits 24..31 represent Multiply(index, 0x%02X).\n"
//                "*/\n", v1, v2, v3, v4);
//
//  fprintf(fout, "static const UINT32 %s[256] =\n{\n", tableName);
//
//  UINT32 c = 0;
//  for(UINT32 i = 0; i < 256; i++)
//  {
//      if(c == 0 )
//          fprintf(fout, "\t");
//
//      UINT32 value = (Multiply(i, v1) & 0xFF) | ((Multiply(i, v2) & 0xFF) << 8) | ((Multiply(i, v3) & 0xFF) << 16) | ((Multiply(i, v4) & 0xFF) << 24);
//
//      if( i != 255 )
//          fprintf(fout, "0x%08X, ", value);
//      else
//          fprintf(fout, "0x%08X", value);
//      if( c == 7 )
//          fprintf(fout, "\n");
//
//      c = (c + 1) & 7;
//  }
//
//  fprintf(fout, "};\n");
//}
//
//void CreateTables(char *fileName)
//{
//  FILE *fout = fopen(fileName, "wt");
//
//  AddTable(fout, "mixColumn_a", 0x02, 0x01, 0x01, 0x03);
//  AddTable(fout, "mixColumn_b", 0x03, 0x02, 0x01, 0x01);
//  AddTable(fout, "mixColumn_c", 0x01, 0x03, 0x02, 0x01);
//  AddTable(fout, "mixColumn_d", 0x01, 0x01, 0x03, 0x02);
//
//  AddTable(fout, "invMixColumn_a", 0x0e, 0x09, 0x0d, 0x0b);
//  AddTable(fout, "invMixColumn_b", 0x0b, 0x0e, 0x09, 0x0d);
//  AddTable(fout, "invMixColumn_c", 0x0d, 0x0b, 0x0e, 0x09);
//  AddTable(fout, "invMixColumn_d", 0x09, 0x0d, 0x0b, 0x0e);
//
//  fclose(fout);
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x02).
        - bits 8..15 represent Multiply(index, 0x01).
        - bits 16..23 represent Multiply(index, 0x01).
        - bits 24..31 represent Multiply(index, 0x03).
*/
__declspec(align(16)) static const UINT32 mixColumn_a[256] =
{
    0x00000000, 0x03010102, 0x06020204, 0x05030306, 0x0C040408, 0x0F05050A, 0x0A06060C, 0x0907070E,
    0x18080810, 0x1B090912, 0x1E0A0A14, 0x1D0B0B16, 0x140C0C18, 0x170D0D1A, 0x120E0E1C, 0x110F0F1E,
    0x30101020, 0x33111122, 0x36121224, 0x35131326, 0x3C141428, 0x3F15152A, 0x3A16162C, 0x3917172E,
    0x28181830, 0x2B191932, 0x2E1A1A34, 0x2D1B1B36, 0x241C1C38, 0x271D1D3A, 0x221E1E3C, 0x211F1F3E,
    0x60202040, 0x63212142, 0x66222244, 0x65232346, 0x6C242448, 0x6F25254A, 0x6A26264C, 0x6927274E,
    0x78282850, 0x7B292952, 0x7E2A2A54, 0x7D2B2B56, 0x742C2C58, 0x772D2D5A, 0x722E2E5C, 0x712F2F5E,
    0x50303060, 0x53313162, 0x56323264, 0x55333366, 0x5C343468, 0x5F35356A, 0x5A36366C, 0x5937376E,
    0x48383870, 0x4B393972, 0x4E3A3A74, 0x4D3B3B76, 0x443C3C78, 0x473D3D7A, 0x423E3E7C, 0x413F3F7E,
    0xC0404080, 0xC3414182, 0xC6424284, 0xC5434386, 0xCC444488, 0xCF45458A, 0xCA46468C, 0xC947478E,
    0xD8484890, 0xDB494992, 0xDE4A4A94, 0xDD4B4B96, 0xD44C4C98, 0xD74D4D9A, 0xD24E4E9C, 0xD14F4F9E,
    0xF05050A0, 0xF35151A2, 0xF65252A4, 0xF55353A6, 0xFC5454A8, 0xFF5555AA, 0xFA5656AC, 0xF95757AE,
    0xE85858B0, 0xEB5959B2, 0xEE5A5AB4, 0xED5B5BB6, 0xE45C5CB8, 0xE75D5DBA, 0xE25E5EBC, 0xE15F5FBE,
    0xA06060C0, 0xA36161C2, 0xA66262C4, 0xA56363C6, 0xAC6464C8, 0xAF6565CA, 0xAA6666CC, 0xA96767CE,
    0xB86868D0, 0xBB6969D2, 0xBE6A6AD4, 0xBD6B6BD6, 0xB46C6CD8, 0xB76D6DDA, 0xB26E6EDC, 0xB16F6FDE,
    0x907070E0, 0x937171E2, 0x967272E4, 0x957373E6, 0x9C7474E8, 0x9F7575EA, 0x9A7676EC, 0x997777EE,
    0x887878F0, 0x8B7979F2, 0x8E7A7AF4, 0x8D7B7BF6, 0x847C7CF8, 0x877D7DFA, 0x827E7EFC, 0x817F7FFE,
    0x9B80801B, 0x98818119, 0x9D82821F, 0x9E83831D, 0x97848413, 0x94858511, 0x91868617, 0x92878715,
    0x8388880B, 0x80898909, 0x858A8A0F, 0x868B8B0D, 0x8F8C8C03, 0x8C8D8D01, 0x898E8E07, 0x8A8F8F05,
    0xAB90903B, 0xA8919139, 0xAD92923F, 0xAE93933D, 0xA7949433, 0xA4959531, 0xA1969637, 0xA2979735,
    0xB398982B, 0xB0999929, 0xB59A9A2F, 0xB69B9B2D, 0xBF9C9C23, 0xBC9D9D21, 0xB99E9E27, 0xBA9F9F25,
    0xFBA0A05B, 0xF8A1A159, 0xFDA2A25F, 0xFEA3A35D, 0xF7A4A453, 0xF4A5A551, 0xF1A6A657, 0xF2A7A755,
    0xE3A8A84B, 0xE0A9A949, 0xE5AAAA4F, 0xE6ABAB4D, 0xEFACAC43, 0xECADAD41, 0xE9AEAE47, 0xEAAFAF45,
    0xCBB0B07B, 0xC8B1B179, 0xCDB2B27F, 0xCEB3B37D, 0xC7B4B473, 0xC4B5B571, 0xC1B6B677, 0xC2B7B775,
    0xD3B8B86B, 0xD0B9B969, 0xD5BABA6F, 0xD6BBBB6D, 0xDFBCBC63, 0xDCBDBD61, 0xD9BEBE67, 0xDABFBF65,
    0x5BC0C09B, 0x58C1C199, 0x5DC2C29F, 0x5EC3C39D, 0x57C4C493, 0x54C5C591, 0x51C6C697, 0x52C7C795,
    0x43C8C88B, 0x40C9C989, 0x45CACA8F, 0x46CBCB8D, 0x4FCCCC83, 0x4CCDCD81, 0x49CECE87, 0x4ACFCF85,
    0x6BD0D0BB, 0x68D1D1B9, 0x6DD2D2BF, 0x6ED3D3BD, 0x67D4D4B3, 0x64D5D5B1, 0x61D6D6B7, 0x62D7D7B5,
    0x73D8D8AB, 0x70D9D9A9, 0x75DADAAF, 0x76DBDBAD, 0x7FDCDCA3, 0x7CDDDDA1, 0x79DEDEA7, 0x7ADFDFA5,
    0x3BE0E0DB, 0x38E1E1D9, 0x3DE2E2DF, 0x3EE3E3DD, 0x37E4E4D3, 0x34E5E5D1, 0x31E6E6D7, 0x32E7E7D5,
    0x23E8E8CB, 0x20E9E9C9, 0x25EAEACF, 0x26EBEBCD, 0x2FECECC3, 0x2CEDEDC1, 0x29EEEEC7, 0x2AEFEFC5,
    0x0BF0F0FB, 0x08F1F1F9, 0x0DF2F2FF, 0x0EF3F3FD, 0x07F4F4F3, 0x04F5F5F1, 0x01F6F6F7, 0x02F7F7F5,
    0x13F8F8EB, 0x10F9F9E9, 0x15FAFAEF, 0x16FBFBED, 0x1FFCFCE3, 0x1CFDFDE1, 0x19FEFEE7, 0x1AFFFFE5
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x03).
        - bits 8..15 represent Multiply(index, 0x02).
        - bits 16..23 represent Multiply(index, 0x01).
        - bits 24..31 represent Multiply(index, 0x01).
*/
__declspec(align(16)) static const UINT32 mixColumn_b[256] =
{
    0x00000000, 0x01010203, 0x02020406, 0x03030605, 0x0404080C, 0x05050A0F, 0x06060C0A, 0x07070E09,
    0x08081018, 0x0909121B, 0x0A0A141E, 0x0B0B161D, 0x0C0C1814, 0x0D0D1A17, 0x0E0E1C12, 0x0F0F1E11,
    0x10102030, 0x11112233, 0x12122436, 0x13132635, 0x1414283C, 0x15152A3F, 0x16162C3A, 0x17172E39,
    0x18183028, 0x1919322B, 0x1A1A342E, 0x1B1B362D, 0x1C1C3824, 0x1D1D3A27, 0x1E1E3C22, 0x1F1F3E21,
    0x20204060, 0x21214263, 0x22224466, 0x23234665, 0x2424486C, 0x25254A6F, 0x26264C6A, 0x27274E69,
    0x28285078, 0x2929527B, 0x2A2A547E, 0x2B2B567D, 0x2C2C5874, 0x2D2D5A77, 0x2E2E5C72, 0x2F2F5E71,
    0x30306050, 0x31316253, 0x32326456, 0x33336655, 0x3434685C, 0x35356A5F, 0x36366C5A, 0x37376E59,
    0x38387048, 0x3939724B, 0x3A3A744E, 0x3B3B764D, 0x3C3C7844, 0x3D3D7A47, 0x3E3E7C42, 0x3F3F7E41,
    0x404080C0, 0x414182C3, 0x424284C6, 0x434386C5, 0x444488CC, 0x45458ACF, 0x46468CCA, 0x47478EC9,
    0x484890D8, 0x494992DB, 0x4A4A94DE, 0x4B4B96DD, 0x4C4C98D4, 0x4D4D9AD7, 0x4E4E9CD2, 0x4F4F9ED1,
    0x5050A0F0, 0x5151A2F3, 0x5252A4F6, 0x5353A6F5, 0x5454A8FC, 0x5555AAFF, 0x5656ACFA, 0x5757AEF9,
    0x5858B0E8, 0x5959B2EB, 0x5A5AB4EE, 0x5B5BB6ED, 0x5C5CB8E4, 0x5D5DBAE7, 0x5E5EBCE2, 0x5F5FBEE1,
    0x6060C0A0, 0x6161C2A3, 0x6262C4A6, 0x6363C6A5, 0x6464C8AC, 0x6565CAAF, 0x6666CCAA, 0x6767CEA9,
    0x6868D0B8, 0x6969D2BB, 0x6A6AD4BE, 0x6B6BD6BD, 0x6C6CD8B4, 0x6D6DDAB7, 0x6E6EDCB2, 0x6F6FDEB1,
    0x7070E090, 0x7171E293, 0x7272E496, 0x7373E695, 0x7474E89C, 0x7575EA9F, 0x7676EC9A, 0x7777EE99,
    0x7878F088, 0x7979F28B, 0x7A7AF48E, 0x7B7BF68D, 0x7C7CF884, 0x7D7DFA87, 0x7E7EFC82, 0x7F7FFE81,
    0x80801B9B, 0x81811998, 0x82821F9D, 0x83831D9E, 0x84841397, 0x85851194, 0x86861791, 0x87871592,
    0x88880B83, 0x89890980, 0x8A8A0F85, 0x8B8B0D86, 0x8C8C038F, 0x8D8D018C, 0x8E8E0789, 0x8F8F058A,
    0x90903BAB, 0x919139A8, 0x92923FAD, 0x93933DAE, 0x949433A7, 0x959531A4, 0x969637A1, 0x979735A2,
    0x98982BB3, 0x999929B0, 0x9A9A2FB5, 0x9B9B2DB6, 0x9C9C23BF, 0x9D9D21BC, 0x9E9E27B9, 0x9F9F25BA,
    0xA0A05BFB, 0xA1A159F8, 0xA2A25FFD, 0xA3A35DFE, 0xA4A453F7, 0xA5A551F4, 0xA6A657F1, 0xA7A755F2,
    0xA8A84BE3, 0xA9A949E0, 0xAAAA4FE5, 0xABAB4DE6, 0xACAC43EF, 0xADAD41EC, 0xAEAE47E9, 0xAFAF45EA,
    0xB0B07BCB, 0xB1B179C8, 0xB2B27FCD, 0xB3B37DCE, 0xB4B473C7, 0xB5B571C4, 0xB6B677C1, 0xB7B775C2,
    0xB8B86BD3, 0xB9B969D0, 0xBABA6FD5, 0xBBBB6DD6, 0xBCBC63DF, 0xBDBD61DC, 0xBEBE67D9, 0xBFBF65DA,
    0xC0C09B5B, 0xC1C19958, 0xC2C29F5D, 0xC3C39D5E, 0xC4C49357, 0xC5C59154, 0xC6C69751, 0xC7C79552,
    0xC8C88B43, 0xC9C98940, 0xCACA8F45, 0xCBCB8D46, 0xCCCC834F, 0xCDCD814C, 0xCECE8749, 0xCFCF854A,
    0xD0D0BB6B, 0xD1D1B968, 0xD2D2BF6D, 0xD3D3BD6E, 0xD4D4B367, 0xD5D5B164, 0xD6D6B761, 0xD7D7B562,
    0xD8D8AB73, 0xD9D9A970, 0xDADAAF75, 0xDBDBAD76, 0xDCDCA37F, 0xDDDDA17C, 0xDEDEA779, 0xDFDFA57A,
    0xE0E0DB3B, 0xE1E1D938, 0xE2E2DF3D, 0xE3E3DD3E, 0xE4E4D337, 0xE5E5D134, 0xE6E6D731, 0xE7E7D532,
    0xE8E8CB23, 0xE9E9C920, 0xEAEACF25, 0xEBEBCD26, 0xECECC32F, 0xEDEDC12C, 0xEEEEC729, 0xEFEFC52A,
    0xF0F0FB0B, 0xF1F1F908, 0xF2F2FF0D, 0xF3F3FD0E, 0xF4F4F307, 0xF5F5F104, 0xF6F6F701, 0xF7F7F502,
    0xF8F8EB13, 0xF9F9E910, 0xFAFAEF15, 0xFBFBED16, 0xFCFCE31F, 0xFDFDE11C, 0xFEFEE719, 0xFFFFE51A
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x01).
        - bits 8..15 represent Multiply(index, 0x03).
        - bits 16..23 represent Multiply(index, 0x02).
        - bits 24..31 represent Multiply(index, 0x01).
*/
__declspec(align(16)) static const UINT32 mixColumn_c[256] =
{
    0x00000000, 0x01020301, 0x02040602, 0x03060503, 0x04080C04, 0x050A0F05, 0x060C0A06, 0x070E0907,
    0x08101808, 0x09121B09, 0x0A141E0A, 0x0B161D0B, 0x0C18140C, 0x0D1A170D, 0x0E1C120E, 0x0F1E110F,
    0x10203010, 0x11223311, 0x12243612, 0x13263513, 0x14283C14, 0x152A3F15, 0x162C3A16, 0x172E3917,
    0x18302818, 0x19322B19, 0x1A342E1A, 0x1B362D1B, 0x1C38241C, 0x1D3A271D, 0x1E3C221E, 0x1F3E211F,
    0x20406020, 0x21426321, 0x22446622, 0x23466523, 0x24486C24, 0x254A6F25, 0x264C6A26, 0x274E6927,
    0x28507828, 0x29527B29, 0x2A547E2A, 0x2B567D2B, 0x2C58742C, 0x2D5A772D, 0x2E5C722E, 0x2F5E712F,
    0x30605030, 0x31625331, 0x32645632, 0x33665533, 0x34685C34, 0x356A5F35, 0x366C5A36, 0x376E5937,
    0x38704838, 0x39724B39, 0x3A744E3A, 0x3B764D3B, 0x3C78443C, 0x3D7A473D, 0x3E7C423E, 0x3F7E413F,
    0x4080C040, 0x4182C341, 0x4284C642, 0x4386C543, 0x4488CC44, 0x458ACF45, 0x468CCA46, 0x478EC947,
    0x4890D848, 0x4992DB49, 0x4A94DE4A, 0x4B96DD4B, 0x4C98D44C, 0x4D9AD74D, 0x4E9CD24E, 0x4F9ED14F,
    0x50A0F050, 0x51A2F351, 0x52A4F652, 0x53A6F553, 0x54A8FC54, 0x55AAFF55, 0x56ACFA56, 0x57AEF957,
    0x58B0E858, 0x59B2EB59, 0x5AB4EE5A, 0x5BB6ED5B, 0x5CB8E45C, 0x5DBAE75D, 0x5EBCE25E, 0x5FBEE15F,
    0x60C0A060, 0x61C2A361, 0x62C4A662, 0x63C6A563, 0x64C8AC64, 0x65CAAF65, 0x66CCAA66, 0x67CEA967,
    0x68D0B868, 0x69D2BB69, 0x6AD4BE6A, 0x6BD6BD6B, 0x6CD8B46C, 0x6DDAB76D, 0x6EDCB26E, 0x6FDEB16F,
    0x70E09070, 0x71E29371, 0x72E49672, 0x73E69573, 0x74E89C74, 0x75EA9F75, 0x76EC9A76, 0x77EE9977,
    0x78F08878, 0x79F28B79, 0x7AF48E7A, 0x7BF68D7B, 0x7CF8847C, 0x7DFA877D, 0x7EFC827E, 0x7FFE817F,
    0x801B9B80, 0x81199881, 0x821F9D82, 0x831D9E83, 0x84139784, 0x85119485, 0x86179186, 0x87159287,
    0x880B8388, 0x89098089, 0x8A0F858A, 0x8B0D868B, 0x8C038F8C, 0x8D018C8D, 0x8E07898E, 0x8F058A8F,
    0x903BAB90, 0x9139A891, 0x923FAD92, 0x933DAE93, 0x9433A794, 0x9531A495, 0x9637A196, 0x9735A297,
    0x982BB398, 0x9929B099, 0x9A2FB59A, 0x9B2DB69B, 0x9C23BF9C, 0x9D21BC9D, 0x9E27B99E, 0x9F25BA9F,
    0xA05BFBA0, 0xA159F8A1, 0xA25FFDA2, 0xA35DFEA3, 0xA453F7A4, 0xA551F4A5, 0xA657F1A6, 0xA755F2A7,
    0xA84BE3A8, 0xA949E0A9, 0xAA4FE5AA, 0xAB4DE6AB, 0xAC43EFAC, 0xAD41ECAD, 0xAE47E9AE, 0xAF45EAAF,
    0xB07BCBB0, 0xB179C8B1, 0xB27FCDB2, 0xB37DCEB3, 0xB473C7B4, 0xB571C4B5, 0xB677C1B6, 0xB775C2B7,
    0xB86BD3B8, 0xB969D0B9, 0xBA6FD5BA, 0xBB6DD6BB, 0xBC63DFBC, 0xBD61DCBD, 0xBE67D9BE, 0xBF65DABF,
    0xC09B5BC0, 0xC19958C1, 0xC29F5DC2, 0xC39D5EC3, 0xC49357C4, 0xC59154C5, 0xC69751C6, 0xC79552C7,
    0xC88B43C8, 0xC98940C9, 0xCA8F45CA, 0xCB8D46CB, 0xCC834FCC, 0xCD814CCD, 0xCE8749CE, 0xCF854ACF,
    0xD0BB6BD0, 0xD1B968D1, 0xD2BF6DD2, 0xD3BD6ED3, 0xD4B367D4, 0xD5B164D5, 0xD6B761D6, 0xD7B562D7,
    0xD8AB73D8, 0xD9A970D9, 0xDAAF75DA, 0xDBAD76DB, 0xDCA37FDC, 0xDDA17CDD, 0xDEA779DE, 0xDFA57ADF,
    0xE0DB3BE0, 0xE1D938E1, 0xE2DF3DE2, 0xE3DD3EE3, 0xE4D337E4, 0xE5D134E5, 0xE6D731E6, 0xE7D532E7,
    0xE8CB23E8, 0xE9C920E9, 0xEACF25EA, 0xEBCD26EB, 0xECC32FEC, 0xEDC12CED, 0xEEC729EE, 0xEFC52AEF,
    0xF0FB0BF0, 0xF1F908F1, 0xF2FF0DF2, 0xF3FD0EF3, 0xF4F307F4, 0xF5F104F5, 0xF6F701F6, 0xF7F502F7,
    0xF8EB13F8, 0xF9E910F9, 0xFAEF15FA, 0xFBED16FB, 0xFCE31FFC, 0xFDE11CFD, 0xFEE719FE, 0xFFE51AFF
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x01).
        - bits 8..15 represent Multiply(index, 0x01).
        - bits 16..23 represent Multiply(index, 0x03).
        - bits 24..31 represent Multiply(index, 0x02).
*/
__declspec(align(16)) static const UINT32 mixColumn_d[256] =
{
    0x00000000, 0x02030101, 0x04060202, 0x06050303, 0x080C0404, 0x0A0F0505, 0x0C0A0606, 0x0E090707,
    0x10180808, 0x121B0909, 0x141E0A0A, 0x161D0B0B, 0x18140C0C, 0x1A170D0D, 0x1C120E0E, 0x1E110F0F,
    0x20301010, 0x22331111, 0x24361212, 0x26351313, 0x283C1414, 0x2A3F1515, 0x2C3A1616, 0x2E391717,
    0x30281818, 0x322B1919, 0x342E1A1A, 0x362D1B1B, 0x38241C1C, 0x3A271D1D, 0x3C221E1E, 0x3E211F1F,
    0x40602020, 0x42632121, 0x44662222, 0x46652323, 0x486C2424, 0x4A6F2525, 0x4C6A2626, 0x4E692727,
    0x50782828, 0x527B2929, 0x547E2A2A, 0x567D2B2B, 0x58742C2C, 0x5A772D2D, 0x5C722E2E, 0x5E712F2F,
    0x60503030, 0x62533131, 0x64563232, 0x66553333, 0x685C3434, 0x6A5F3535, 0x6C5A3636, 0x6E593737,
    0x70483838, 0x724B3939, 0x744E3A3A, 0x764D3B3B, 0x78443C3C, 0x7A473D3D, 0x7C423E3E, 0x7E413F3F,
    0x80C04040, 0x82C34141, 0x84C64242, 0x86C54343, 0x88CC4444, 0x8ACF4545, 0x8CCA4646, 0x8EC94747,
    0x90D84848, 0x92DB4949, 0x94DE4A4A, 0x96DD4B4B, 0x98D44C4C, 0x9AD74D4D, 0x9CD24E4E, 0x9ED14F4F,
    0xA0F05050, 0xA2F35151, 0xA4F65252, 0xA6F55353, 0xA8FC5454, 0xAAFF5555, 0xACFA5656, 0xAEF95757,
    0xB0E85858, 0xB2EB5959, 0xB4EE5A5A, 0xB6ED5B5B, 0xB8E45C5C, 0xBAE75D5D, 0xBCE25E5E, 0xBEE15F5F,
    0xC0A06060, 0xC2A36161, 0xC4A66262, 0xC6A56363, 0xC8AC6464, 0xCAAF6565, 0xCCAA6666, 0xCEA96767,
    0xD0B86868, 0xD2BB6969, 0xD4BE6A6A, 0xD6BD6B6B, 0xD8B46C6C, 0xDAB76D6D, 0xDCB26E6E, 0xDEB16F6F,
    0xE0907070, 0xE2937171, 0xE4967272, 0xE6957373, 0xE89C7474, 0xEA9F7575, 0xEC9A7676, 0xEE997777,
    0xF0887878, 0xF28B7979, 0xF48E7A7A, 0xF68D7B7B, 0xF8847C7C, 0xFA877D7D, 0xFC827E7E, 0xFE817F7F,
    0x1B9B8080, 0x19988181, 0x1F9D8282, 0x1D9E8383, 0x13978484, 0x11948585, 0x17918686, 0x15928787,
    0x0B838888, 0x09808989, 0x0F858A8A, 0x0D868B8B, 0x038F8C8C, 0x018C8D8D, 0x07898E8E, 0x058A8F8F,
    0x3BAB9090, 0x39A89191, 0x3FAD9292, 0x3DAE9393, 0x33A79494, 0x31A49595, 0x37A19696, 0x35A29797,
    0x2BB39898, 0x29B09999, 0x2FB59A9A, 0x2DB69B9B, 0x23BF9C9C, 0x21BC9D9D, 0x27B99E9E, 0x25BA9F9F,
    0x5BFBA0A0, 0x59F8A1A1, 0x5FFDA2A2, 0x5DFEA3A3, 0x53F7A4A4, 0x51F4A5A5, 0x57F1A6A6, 0x55F2A7A7,
    0x4BE3A8A8, 0x49E0A9A9, 0x4FE5AAAA, 0x4DE6ABAB, 0x43EFACAC, 0x41ECADAD, 0x47E9AEAE, 0x45EAAFAF,
    0x7BCBB0B0, 0x79C8B1B1, 0x7FCDB2B2, 0x7DCEB3B3, 0x73C7B4B4, 0x71C4B5B5, 0x77C1B6B6, 0x75C2B7B7,
    0x6BD3B8B8, 0x69D0B9B9, 0x6FD5BABA, 0x6DD6BBBB, 0x63DFBCBC, 0x61DCBDBD, 0x67D9BEBE, 0x65DABFBF,
    0x9B5BC0C0, 0x9958C1C1, 0x9F5DC2C2, 0x9D5EC3C3, 0x9357C4C4, 0x9154C5C5, 0x9751C6C6, 0x9552C7C7,
    0x8B43C8C8, 0x8940C9C9, 0x8F45CACA, 0x8D46CBCB, 0x834FCCCC, 0x814CCDCD, 0x8749CECE, 0x854ACFCF,
    0xBB6BD0D0, 0xB968D1D1, 0xBF6DD2D2, 0xBD6ED3D3, 0xB367D4D4, 0xB164D5D5, 0xB761D6D6, 0xB562D7D7,
    0xAB73D8D8, 0xA970D9D9, 0xAF75DADA, 0xAD76DBDB, 0xA37FDCDC, 0xA17CDDDD, 0xA779DEDE, 0xA57ADFDF,
    0xDB3BE0E0, 0xD938E1E1, 0xDF3DE2E2, 0xDD3EE3E3, 0xD337E4E4, 0xD134E5E5, 0xD731E6E6, 0xD532E7E7,
    0xCB23E8E8, 0xC920E9E9, 0xCF25EAEA, 0xCD26EBEB, 0xC32FECEC, 0xC12CEDED, 0xC729EEEE, 0xC52AEFEF,
    0xFB0BF0F0, 0xF908F1F1, 0xFF0DF2F2, 0xFD0EF3F3, 0xF307F4F4, 0xF104F5F5, 0xF701F6F6, 0xF502F7F7,
    0xEB13F8F8, 0xE910F9F9, 0xEF15FAFA, 0xED16FBFB, 0xE31FFCFC, 0xE11CFDFD, 0xE719FEFE, 0xE51AFFFF
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x0E).
        - bits 8..15 represent Multiply(index, 0x09).
        - bits 16..23 represent Multiply(index, 0x0D).
        - bits 24..31 represent Multiply(index, 0x0B).
*/
__declspec(align(16)) static const UINT32 invMixColumn_a[256] =
{
    0x00000000, 0x0B0D090E, 0x161A121C, 0x1D171B12, 0x2C342438, 0x27392D36, 0x3A2E3624, 0x31233F2A,
    0x58684870, 0x5365417E, 0x4E725A6C, 0x457F5362, 0x745C6C48, 0x7F516546, 0x62467E54, 0x694B775A,
    0xB0D090E0, 0xBBDD99EE, 0xA6CA82FC, 0xADC78BF2, 0x9CE4B4D8, 0x97E9BDD6, 0x8AFEA6C4, 0x81F3AFCA,
    0xE8B8D890, 0xE3B5D19E, 0xFEA2CA8C, 0xF5AFC382, 0xC48CFCA8, 0xCF81F5A6, 0xD296EEB4, 0xD99BE7BA,
    0x7BBB3BDB, 0x70B632D5, 0x6DA129C7, 0x66AC20C9, 0x578F1FE3, 0x5C8216ED, 0x41950DFF, 0x4A9804F1,
    0x23D373AB, 0x28DE7AA5, 0x35C961B7, 0x3EC468B9, 0x0FE75793, 0x04EA5E9D, 0x19FD458F, 0x12F04C81,
    0xCB6BAB3B, 0xC066A235, 0xDD71B927, 0xD67CB029, 0xE75F8F03, 0xEC52860D, 0xF1459D1F, 0xFA489411,
    0x9303E34B, 0x980EEA45, 0x8519F157, 0x8E14F859, 0xBF37C773, 0xB43ACE7D, 0xA92DD56F, 0xA220DC61,
    0xF66D76AD, 0xFD607FA3, 0xE07764B1, 0xEB7A6DBF, 0xDA595295, 0xD1545B9B, 0xCC434089, 0xC74E4987,
    0xAE053EDD, 0xA50837D3, 0xB81F2CC1, 0xB31225CF, 0x82311AE5, 0x893C13EB, 0x942B08F9, 0x9F2601F7,
    0x46BDE64D, 0x4DB0EF43, 0x50A7F451, 0x5BAAFD5F, 0x6A89C275, 0x6184CB7B, 0x7C93D069, 0x779ED967,
    0x1ED5AE3D, 0x15D8A733, 0x08CFBC21, 0x03C2B52F, 0x32E18A05, 0x39EC830B, 0x24FB9819, 0x2FF69117,
    0x8DD64D76, 0x86DB4478, 0x9BCC5F6A, 0x90C15664, 0xA1E2694E, 0xAAEF6040, 0xB7F87B52, 0xBCF5725C,
    0xD5BE0506, 0xDEB30C08, 0xC3A4171A, 0xC8A91E14, 0xF98A213E, 0xF2872830, 0xEF903322, 0xE49D3A2C,
    0x3D06DD96, 0x360BD498, 0x2B1CCF8A, 0x2011C684, 0x1132F9AE, 0x1A3FF0A0, 0x0728EBB2, 0x0C25E2BC,
    0x656E95E6, 0x6E639CE8, 0x737487FA, 0x78798EF4, 0x495AB1DE, 0x4257B8D0, 0x5F40A3C2, 0x544DAACC,
    0xF7DAEC41, 0xFCD7E54F, 0xE1C0FE5D, 0xEACDF753, 0xDBEEC879, 0xD0E3C177, 0xCDF4DA65, 0xC6F9D36B,
    0xAFB2A431, 0xA4BFAD3F, 0xB9A8B62D, 0xB2A5BF23, 0x83868009, 0x888B8907, 0x959C9215, 0x9E919B1B,
    0x470A7CA1, 0x4C0775AF, 0x51106EBD, 0x5A1D67B3, 0x6B3E5899, 0x60335197, 0x7D244A85, 0x7629438B,
    0x1F6234D1, 0x146F3DDF, 0x097826CD, 0x02752FC3, 0x335610E9, 0x385B19E7, 0x254C02F5, 0x2E410BFB,
    0x8C61D79A, 0x876CDE94, 0x9A7BC586, 0x9176CC88, 0xA055F3A2, 0xAB58FAAC, 0xB64FE1BE, 0xBD42E8B0,
    0xD4099FEA, 0xDF0496E4, 0xC2138DF6, 0xC91E84F8, 0xF83DBBD2, 0xF330B2DC, 0xEE27A9CE, 0xE52AA0C0,
    0x3CB1477A, 0x37BC4E74, 0x2AAB5566, 0x21A65C68, 0x10856342, 0x1B886A4C, 0x069F715E, 0x0D927850,
    0x64D90F0A, 0x6FD40604, 0x72C31D16, 0x79CE1418, 0x48ED2B32, 0x43E0223C, 0x5EF7392E, 0x55FA3020,
    0x01B79AEC, 0x0ABA93E2, 0x17AD88F0, 0x1CA081FE, 0x2D83BED4, 0x268EB7DA, 0x3B99ACC8, 0x3094A5C6,
    0x59DFD29C, 0x52D2DB92, 0x4FC5C080, 0x44C8C98E, 0x75EBF6A4, 0x7EE6FFAA, 0x63F1E4B8, 0x68FCEDB6,
    0xB1670A0C, 0xBA6A0302, 0xA77D1810, 0xAC70111E, 0x9D532E34, 0x965E273A, 0x8B493C28, 0x80443526,
    0xE90F427C, 0xE2024B72, 0xFF155060, 0xF418596E, 0xC53B6644, 0xCE366F4A, 0xD3217458, 0xD82C7D56,
    0x7A0CA137, 0x7101A839, 0x6C16B32B, 0x671BBA25, 0x5638850F, 0x5D358C01, 0x40229713, 0x4B2F9E1D,
    0x2264E947, 0x2969E049, 0x347EFB5B, 0x3F73F255, 0x0E50CD7F, 0x055DC471, 0x184ADF63, 0x1347D66D,
    0xCADC31D7, 0xC1D138D9, 0xDCC623CB, 0xD7CB2AC5, 0xE6E815EF, 0xEDE51CE1, 0xF0F207F3, 0xFBFF0EFD,
    0x92B479A7, 0x99B970A9, 0x84AE6BBB, 0x8FA362B5, 0xBE805D9F, 0xB58D5491, 0xA89A4F83, 0xA397468D
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x0B).
        - bits 8..15 represent Multiply(index, 0x0E).
        - bits 16..23 represent Multiply(index, 0x09).
        - bits 24..31 represent Multiply(index, 0x0D).
*/
__declspec(align(16)) static const UINT32 invMixColumn_b[256] =
{
    0x00000000, 0x0D090E0B, 0x1A121C16, 0x171B121D, 0x3424382C, 0x392D3627, 0x2E36243A, 0x233F2A31,
    0x68487058, 0x65417E53, 0x725A6C4E, 0x7F536245, 0x5C6C4874, 0x5165467F, 0x467E5462, 0x4B775A69,
    0xD090E0B0, 0xDD99EEBB, 0xCA82FCA6, 0xC78BF2AD, 0xE4B4D89C, 0xE9BDD697, 0xFEA6C48A, 0xF3AFCA81,
    0xB8D890E8, 0xB5D19EE3, 0xA2CA8CFE, 0xAFC382F5, 0x8CFCA8C4, 0x81F5A6CF, 0x96EEB4D2, 0x9BE7BAD9,
    0xBB3BDB7B, 0xB632D570, 0xA129C76D, 0xAC20C966, 0x8F1FE357, 0x8216ED5C, 0x950DFF41, 0x9804F14A,
    0xD373AB23, 0xDE7AA528, 0xC961B735, 0xC468B93E, 0xE757930F, 0xEA5E9D04, 0xFD458F19, 0xF04C8112,
    0x6BAB3BCB, 0x66A235C0, 0x71B927DD, 0x7CB029D6, 0x5F8F03E7, 0x52860DEC, 0x459D1FF1, 0x489411FA,
    0x03E34B93, 0x0EEA4598, 0x19F15785, 0x14F8598E, 0x37C773BF, 0x3ACE7DB4, 0x2DD56FA9, 0x20DC61A2,
    0x6D76ADF6, 0x607FA3FD, 0x7764B1E0, 0x7A6DBFEB, 0x595295DA, 0x545B9BD1, 0x434089CC, 0x4E4987C7,
    0x053EDDAE, 0x0837D3A5, 0x1F2CC1B8, 0x1225CFB3, 0x311AE582, 0x3C13EB89, 0x2B08F994, 0x2601F79F,
    0xBDE64D46, 0xB0EF434D, 0xA7F45150, 0xAAFD5F5B, 0x89C2756A, 0x84CB7B61, 0x93D0697C, 0x9ED96777,
    0xD5AE3D1E, 0xD8A73315, 0xCFBC2108, 0xC2B52F03, 0xE18A0532, 0xEC830B39, 0xFB981924, 0xF691172F,
    0xD64D768D, 0xDB447886, 0xCC5F6A9B, 0xC1566490, 0xE2694EA1, 0xEF6040AA, 0xF87B52B7, 0xF5725CBC,
    0xBE0506D5, 0xB30C08DE, 0xA4171AC3, 0xA91E14C8, 0x8A213EF9, 0x872830F2, 0x903322EF, 0x9D3A2CE4,
    0x06DD963D, 0x0BD49836, 0x1CCF8A2B, 0x11C68420, 0x32F9AE11, 0x3FF0A01A, 0x28EBB207, 0x25E2BC0C,
    0x6E95E665, 0x639CE86E, 0x7487FA73, 0x798EF478, 0x5AB1DE49, 0x57B8D042, 0x40A3C25F, 0x4DAACC54,
    0xDAEC41F7, 0xD7E54FFC, 0xC0FE5DE1, 0xCDF753EA, 0xEEC879DB, 0xE3C177D0, 0xF4DA65CD, 0xF9D36BC6,
    0xB2A431AF, 0xBFAD3FA4, 0xA8B62DB9, 0xA5BF23B2, 0x86800983, 0x8B890788, 0x9C921595, 0x919B1B9E,
    0x0A7CA147, 0x0775AF4C, 0x106EBD51, 0x1D67B35A, 0x3E58996B, 0x33519760, 0x244A857D, 0x29438B76,
    0x6234D11F, 0x6F3DDF14, 0x7826CD09, 0x752FC302, 0x5610E933, 0x5B19E738, 0x4C02F525, 0x410BFB2E,
    0x61D79A8C, 0x6CDE9487, 0x7BC5869A, 0x76CC8891, 0x55F3A2A0, 0x58FAACAB, 0x4FE1BEB6, 0x42E8B0BD,
    0x099FEAD4, 0x0496E4DF, 0x138DF6C2, 0x1E84F8C9, 0x3DBBD2F8, 0x30B2DCF3, 0x27A9CEEE, 0x2AA0C0E5,
    0xB1477A3C, 0xBC4E7437, 0xAB55662A, 0xA65C6821, 0x85634210, 0x886A4C1B, 0x9F715E06, 0x9278500D,
    0xD90F0A64, 0xD406046F, 0xC31D1672, 0xCE141879, 0xED2B3248, 0xE0223C43, 0xF7392E5E, 0xFA302055,
    0xB79AEC01, 0xBA93E20A, 0xAD88F017, 0xA081FE1C, 0x83BED42D, 0x8EB7DA26, 0x99ACC83B, 0x94A5C630,
    0xDFD29C59, 0xD2DB9252, 0xC5C0804F, 0xC8C98E44, 0xEBF6A475, 0xE6FFAA7E, 0xF1E4B863, 0xFCEDB668,
    0x670A0CB1, 0x6A0302BA, 0x7D1810A7, 0x70111EAC, 0x532E349D, 0x5E273A96, 0x493C288B, 0x44352680,
    0x0F427CE9, 0x024B72E2, 0x155060FF, 0x18596EF4, 0x3B6644C5, 0x366F4ACE, 0x217458D3, 0x2C7D56D8,
    0x0CA1377A, 0x01A83971, 0x16B32B6C, 0x1BBA2567, 0x38850F56, 0x358C015D, 0x22971340, 0x2F9E1D4B,
    0x64E94722, 0x69E04929, 0x7EFB5B34, 0x73F2553F, 0x50CD7F0E, 0x5DC47105, 0x4ADF6318, 0x47D66D13,
    0xDC31D7CA, 0xD138D9C1, 0xC623CBDC, 0xCB2AC5D7, 0xE815EFE6, 0xE51CE1ED, 0xF207F3F0, 0xFF0EFDFB,
    0xB479A792, 0xB970A999, 0xAE6BBB84, 0xA362B58F, 0x805D9FBE, 0x8D5491B5, 0x9A4F83A8, 0x97468DA3
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x0D).
        - bits 8..15 represent Multiply(index, 0x0B).
        - bits 16..23 represent Multiply(index, 0x0E).
        - bits 24..31 represent Multiply(index, 0x09).
*/
__declspec(align(16)) static const UINT32 invMixColumn_c[256] =
{
    0x00000000, 0x090E0B0D, 0x121C161A, 0x1B121D17, 0x24382C34, 0x2D362739, 0x36243A2E, 0x3F2A3123,
    0x48705868, 0x417E5365, 0x5A6C4E72, 0x5362457F, 0x6C48745C, 0x65467F51, 0x7E546246, 0x775A694B,
    0x90E0B0D0, 0x99EEBBDD, 0x82FCA6CA, 0x8BF2ADC7, 0xB4D89CE4, 0xBDD697E9, 0xA6C48AFE, 0xAFCA81F3,
    0xD890E8B8, 0xD19EE3B5, 0xCA8CFEA2, 0xC382F5AF, 0xFCA8C48C, 0xF5A6CF81, 0xEEB4D296, 0xE7BAD99B,
    0x3BDB7BBB, 0x32D570B6, 0x29C76DA1, 0x20C966AC, 0x1FE3578F, 0x16ED5C82, 0x0DFF4195, 0x04F14A98,
    0x73AB23D3, 0x7AA528DE, 0x61B735C9, 0x68B93EC4, 0x57930FE7, 0x5E9D04EA, 0x458F19FD, 0x4C8112F0,
    0xAB3BCB6B, 0xA235C066, 0xB927DD71, 0xB029D67C, 0x8F03E75F, 0x860DEC52, 0x9D1FF145, 0x9411FA48,
    0xE34B9303, 0xEA45980E, 0xF1578519, 0xF8598E14, 0xC773BF37, 0xCE7DB43A, 0xD56FA92D, 0xDC61A220,
    0x76ADF66D, 0x7FA3FD60, 0x64B1E077, 0x6DBFEB7A, 0x5295DA59, 0x5B9BD154, 0x4089CC43, 0x4987C74E,
    0x3EDDAE05, 0x37D3A508, 0x2CC1B81F, 0x25CFB312, 0x1AE58231, 0x13EB893C, 0x08F9942B, 0x01F79F26,
    0xE64D46BD, 0xEF434DB0, 0xF45150A7, 0xFD5F5BAA, 0xC2756A89, 0xCB7B6184, 0xD0697C93, 0xD967779E,
    0xAE3D1ED5, 0xA73315D8, 0xBC2108CF, 0xB52F03C2, 0x8A0532E1, 0x830B39EC, 0x981924FB, 0x91172FF6,
    0x4D768DD6, 0x447886DB, 0x5F6A9BCC, 0x566490C1, 0x694EA1E2, 0x6040AAEF, 0x7B52B7F8, 0x725CBCF5,
    0x0506D5BE, 0x0C08DEB3, 0x171AC3A4, 0x1E14C8A9, 0x213EF98A, 0x2830F287, 0x3322EF90, 0x3A2CE49D,
    0xDD963D06, 0xD498360B, 0xCF8A2B1C, 0xC6842011, 0xF9AE1132, 0xF0A01A3F, 0xEBB20728, 0xE2BC0C25,
    0x95E6656E, 0x9CE86E63, 0x87FA7374, 0x8EF47879, 0xB1DE495A, 0xB8D04257, 0xA3C25F40, 0xAACC544D,
    0xEC41F7DA, 0xE54FFCD7, 0xFE5DE1C0, 0xF753EACD, 0xC879DBEE, 0xC177D0E3, 0xDA65CDF4, 0xD36BC6F9,
    0xA431AFB2, 0xAD3FA4BF, 0xB62DB9A8, 0xBF23B2A5, 0x80098386, 0x8907888B, 0x9215959C, 0x9B1B9E91,
    0x7CA1470A, 0x75AF4C07, 0x6EBD5110, 0x67B35A1D, 0x58996B3E, 0x51976033, 0x4A857D24, 0x438B7629,
    0x34D11F62, 0x3DDF146F, 0x26CD0978, 0x2FC30275, 0x10E93356, 0x19E7385B, 0x02F5254C, 0x0BFB2E41,
    0xD79A8C61, 0xDE94876C, 0xC5869A7B, 0xCC889176, 0xF3A2A055, 0xFAACAB58, 0xE1BEB64F, 0xE8B0BD42,
    0x9FEAD409, 0x96E4DF04, 0x8DF6C213, 0x84F8C91E, 0xBBD2F83D, 0xB2DCF330, 0xA9CEEE27, 0xA0C0E52A,
    0x477A3CB1, 0x4E7437BC, 0x55662AAB, 0x5C6821A6, 0x63421085, 0x6A4C1B88, 0x715E069F, 0x78500D92,
    0x0F0A64D9, 0x06046FD4, 0x1D1672C3, 0x141879CE, 0x2B3248ED, 0x223C43E0, 0x392E5EF7, 0x302055FA,
    0x9AEC01B7, 0x93E20ABA, 0x88F017AD, 0x81FE1CA0, 0xBED42D83, 0xB7DA268E, 0xACC83B99, 0xA5C63094,
    0xD29C59DF, 0xDB9252D2, 0xC0804FC5, 0xC98E44C8, 0xF6A475EB, 0xFFAA7EE6, 0xE4B863F1, 0xEDB668FC,
    0x0A0CB167, 0x0302BA6A, 0x1810A77D, 0x111EAC70, 0x2E349D53, 0x273A965E, 0x3C288B49, 0x35268044,
    0x427CE90F, 0x4B72E202, 0x5060FF15, 0x596EF418, 0x6644C53B, 0x6F4ACE36, 0x7458D321, 0x7D56D82C,
    0xA1377A0C, 0xA8397101, 0xB32B6C16, 0xBA25671B, 0x850F5638, 0x8C015D35, 0x97134022, 0x9E1D4B2F,
    0xE9472264, 0xE0492969, 0xFB5B347E, 0xF2553F73, 0xCD7F0E50, 0xC471055D, 0xDF63184A, 0xD66D1347,
    0x31D7CADC, 0x38D9C1D1, 0x23CBDCC6, 0x2AC5D7CB, 0x15EFE6E8, 0x1CE1EDE5, 0x07F3F0F2, 0x0EFDFBFF,
    0x79A792B4, 0x70A999B9, 0x6BBB84AE, 0x62B58FA3, 0x5D9FBE80, 0x5491B58D, 0x4F83A89A, 0x468DA397
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    For every value found at index we have:
        - bits 0..7 represent Multiply(index, 0x09).
        - bits 8..15 represent Multiply(index, 0x0D).
        - bits 16..23 represent Multiply(index, 0x0B).
        - bits 24..31 represent Multiply(index, 0x0E).
*/
__declspec(align(16)) static const UINT32 invMixColumn_d[256] =
{
    0x00000000, 0x0E0B0D09, 0x1C161A12, 0x121D171B, 0x382C3424, 0x3627392D, 0x243A2E36, 0x2A31233F,
    0x70586848, 0x7E536541, 0x6C4E725A, 0x62457F53, 0x48745C6C, 0x467F5165, 0x5462467E, 0x5A694B77,
    0xE0B0D090, 0xEEBBDD99, 0xFCA6CA82, 0xF2ADC78B, 0xD89CE4B4, 0xD697E9BD, 0xC48AFEA6, 0xCA81F3AF,
    0x90E8B8D8, 0x9EE3B5D1, 0x8CFEA2CA, 0x82F5AFC3, 0xA8C48CFC, 0xA6CF81F5, 0xB4D296EE, 0xBAD99BE7,
    0xDB7BBB3B, 0xD570B632, 0xC76DA129, 0xC966AC20, 0xE3578F1F, 0xED5C8216, 0xFF41950D, 0xF14A9804,
    0xAB23D373, 0xA528DE7A, 0xB735C961, 0xB93EC468, 0x930FE757, 0x9D04EA5E, 0x8F19FD45, 0x8112F04C,
    0x3BCB6BAB, 0x35C066A2, 0x27DD71B9, 0x29D67CB0, 0x03E75F8F, 0x0DEC5286, 0x1FF1459D, 0x11FA4894,
    0x4B9303E3, 0x45980EEA, 0x578519F1, 0x598E14F8, 0x73BF37C7, 0x7DB43ACE, 0x6FA92DD5, 0x61A220DC,
    0xADF66D76, 0xA3FD607F, 0xB1E07764, 0xBFEB7A6D, 0x95DA5952, 0x9BD1545B, 0x89CC4340, 0x87C74E49,
    0xDDAE053E, 0xD3A50837, 0xC1B81F2C, 0xCFB31225, 0xE582311A, 0xEB893C13, 0xF9942B08, 0xF79F2601,
    0x4D46BDE6, 0x434DB0EF, 0x5150A7F4, 0x5F5BAAFD, 0x756A89C2, 0x7B6184CB, 0x697C93D0, 0x67779ED9,
    0x3D1ED5AE, 0x3315D8A7, 0x2108CFBC, 0x2F03C2B5, 0x0532E18A, 0x0B39EC83, 0x1924FB98, 0x172FF691,
    0x768DD64D, 0x7886DB44, 0x6A9BCC5F, 0x6490C156, 0x4EA1E269, 0x40AAEF60, 0x52B7F87B, 0x5CBCF572,
    0x06D5BE05, 0x08DEB30C, 0x1AC3A417, 0x14C8A91E, 0x3EF98A21, 0x30F28728, 0x22EF9033, 0x2CE49D3A,
    0x963D06DD, 0x98360BD4, 0x8A2B1CCF, 0x842011C6, 0xAE1132F9, 0xA01A3FF0, 0xB20728EB, 0xBC0C25E2,
    0xE6656E95, 0xE86E639C, 0xFA737487, 0xF478798E, 0xDE495AB1, 0xD04257B8, 0xC25F40A3, 0xCC544DAA,
    0x41F7DAEC, 0x4FFCD7E5, 0x5DE1C0FE, 0x53EACDF7, 0x79DBEEC8, 0x77D0E3C1, 0x65CDF4DA, 0x6BC6F9D3,
    0x31AFB2A4, 0x3FA4BFAD, 0x2DB9A8B6, 0x23B2A5BF, 0x09838680, 0x07888B89, 0x15959C92, 0x1B9E919B,
    0xA1470A7C, 0xAF4C0775, 0xBD51106E, 0xB35A1D67, 0x996B3E58, 0x97603351, 0x857D244A, 0x8B762943,
    0xD11F6234, 0xDF146F3D, 0xCD097826, 0xC302752F, 0xE9335610, 0xE7385B19, 0xF5254C02, 0xFB2E410B,
    0x9A8C61D7, 0x94876CDE, 0x869A7BC5, 0x889176CC, 0xA2A055F3, 0xACAB58FA, 0xBEB64FE1, 0xB0BD42E8,
    0xEAD4099F, 0xE4DF0496, 0xF6C2138D, 0xF8C91E84, 0xD2F83DBB, 0xDCF330B2, 0xCEEE27A9, 0xC0E52AA0,
    0x7A3CB147, 0x7437BC4E, 0x662AAB55, 0x6821A65C, 0x42108563, 0x4C1B886A, 0x5E069F71, 0x500D9278,
    0x0A64D90F, 0x046FD406, 0x1672C31D, 0x1879CE14, 0x3248ED2B, 0x3C43E022, 0x2E5EF739, 0x2055FA30,
    0xEC01B79A, 0xE20ABA93, 0xF017AD88, 0xFE1CA081, 0xD42D83BE, 0xDA268EB7, 0xC83B99AC, 0xC63094A5,
    0x9C59DFD2, 0x9252D2DB, 0x804FC5C0, 0x8E44C8C9, 0xA475EBF6, 0xAA7EE6FF, 0xB863F1E4, 0xB668FCED,
    0x0CB1670A, 0x02BA6A03, 0x10A77D18, 0x1EAC7011, 0x349D532E, 0x3A965E27, 0x288B493C, 0x26804435,
    0x7CE90F42, 0x72E2024B, 0x60FF1550, 0x6EF41859, 0x44C53B66, 0x4ACE366F, 0x58D32174, 0x56D82C7D,
    0x377A0CA1, 0x397101A8, 0x2B6C16B3, 0x25671BBA, 0x0F563885, 0x015D358C, 0x13402297, 0x1D4B2F9E,
    0x472264E9, 0x492969E0, 0x5B347EFB, 0x553F73F2, 0x7F0E50CD, 0x71055DC4, 0x63184ADF, 0x6D1347D6,
    0xD7CADC31, 0xD9C1D138, 0xCBDCC623, 0xC5D7CB2A, 0xEFE6E815, 0xE1EDE51C, 0xF3F0F207, 0xFDFBFF0E,
    0xA792B479, 0xA999B970, 0xBB84AE6B, 0xB58FA362, 0x9FBE805D, 0x91B58D54, 0x83A89A4F, 0x8DA39746
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_CPU_Impl_O1::SetKeyData(const UINT8 *keyData, UINT32 keyDataSize, AES_Key::AES_KeySize keySize)
{
    m_keySize = keySize;
    m_Nk = m_keySize >> 5;
    m_Nr = m_Nk + 6;
    memcpy(m_keyEnc, keyData, min(keyDataSize, countof(m_keyEnc)));

    // Because we want to use the same form of algorithm for decryption as for encryption, we
    // copy in m_keyDec the keys in reverse order of the rounds and also apply InvMixColumns 
    // on the middle round keys . 
    // For more information see Equivalent Inverse Cipher on AES standard.
    UINT32 *src = (UINT32*)&m_keyEnc[m_Nr * 16];
    UINT32 *dst = (UINT32*)m_keyDec;

    // First round
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src;
    src -= 7;

    // The rest of the rounds will have applied the InvMixColumns to avoid being done on DecryptBlock
    for(UINT32 i = 1; i < m_Nr; i++)
    {
        UINT32 value = *src++;
        *dst++ = invMixColumn_a[value & 0xFF]           ^
                 invMixColumn_b[(value >> 8) & 0xFF]    ^
                 invMixColumn_c[(value >> 16) & 0xFF]   ^
                 invMixColumn_d[value >> 24];

        value = *src++;
        *dst++ = invMixColumn_a[value & 0xFF]           ^
                 invMixColumn_b[(value >> 8) & 0xFF]    ^
                 invMixColumn_c[(value >> 16) & 0xFF]   ^
                 invMixColumn_d[value >> 24];

        value = *src++;
        *dst++ = invMixColumn_a[value & 0xFF]           ^
                 invMixColumn_b[(value >> 8) & 0xFF]    ^
                 invMixColumn_c[(value >> 16) & 0xFF]   ^
                 invMixColumn_d[value >> 24];

        value = *src;
        *dst++ = invMixColumn_a[value & 0xFF]           ^
                 invMixColumn_b[(value >> 8) & 0xFF]    ^
                 invMixColumn_c[(value >> 16) & 0xFF]   ^
                 invMixColumn_d[value >> 24];

        src -= 7;
    }

    // The last round
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst = *src;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_CPU_Impl_O1::EncryptBlock(UINT8 *dst, const UINT8 *src)
{
    UINT32 *r_key = (UINT32*)m_keyEnc,
           *src32 = (UINT32*)src,
           *dst32 = (UINT32*)dst;
    UINT8 m_state[4][4];
    UINT32 value;

    // First round will copy the data in the state and add the round key
    value = src32[0] ^ r_key[0];
    m_state[0][0] = (UINT8)value;
    m_state[1][0] = (UINT8)(value >> 8);
    m_state[2][0] = (UINT8)(value >> 16);
    m_state[3][0] = (UINT8)(value >> 24);

    value = src32[1] ^ r_key[1];
    m_state[0][1] = (UINT8)value;
    m_state[1][1] = (UINT8)(value >> 8);
    m_state[2][1] = (UINT8)(value >> 16);
    m_state[3][1] = (UINT8)(value >> 24);

    value = src32[2] ^ r_key[2];
    m_state[0][2] = (UINT8)value;
    m_state[1][2] = (UINT8)(value >> 8);
    m_state[2][2] = (UINT8)(value >> 16);
    m_state[3][2] = (UINT8)(value >> 24);

    value = src32[3] ^ r_key[3];
    m_state[0][3] = (UINT8)value;
    m_state[1][3] = (UINT8)(value >> 8);
    m_state[2][3] = (UINT8)(value >> 16);
    m_state[3][3] = (UINT8)(value >> 24);

    // Do the rest of the rounds
    r_key += 4;
    UINT32 round = m_Nr - 1;
    do 
    {
        UINT8 r_state[4][4];
        // Make SubBytes and ShiftRows in the same step and put the result in r_state
        r_state[0][0] = sbox[m_state[0][0]];
        r_state[0][1] = sbox[m_state[0][1]];
        r_state[0][2] = sbox[m_state[0][2]];
        r_state[0][3] = sbox[m_state[0][3]];

        r_state[1][0] = sbox[m_state[1][1]];
        r_state[1][1] = sbox[m_state[1][2]];
        r_state[1][2] = sbox[m_state[1][3]];
        r_state[1][3] = sbox[m_state[1][0]];

        r_state[2][0] = sbox[m_state[2][2]];
        r_state[2][1] = sbox[m_state[2][3]];
        r_state[2][2] = sbox[m_state[2][0]];
        r_state[2][3] = sbox[m_state[2][1]];

        r_state[3][0] = sbox[m_state[3][3]];
        r_state[3][1] = sbox[m_state[3][0]];
        r_state[3][2] = sbox[m_state[3][1]];
        r_state[3][3] = sbox[m_state[3][2]];

        // Make MixColumns and AddRoundKey in the same step and put the result back in m_state
        value = mixColumn_a[r_state[0][0]] ^
                mixColumn_b[r_state[1][0]] ^
                mixColumn_c[r_state[2][0]] ^
                mixColumn_d[r_state[3][0]] ^
                r_key[0];
        m_state[0][0] = (UINT8)value;
        m_state[1][0] = (UINT8)(value >> 8);
        m_state[2][0] = (UINT8)(value >> 16);
        m_state[3][0] = (UINT8)(value >> 24);

        value = mixColumn_a[r_state[0][1]] ^
                mixColumn_b[r_state[1][1]] ^
                mixColumn_c[r_state[2][1]] ^
                mixColumn_d[r_state[3][1]] ^
                r_key[1];
        m_state[0][1] = (UINT8)value;
        m_state[1][1] = (UINT8)(value >> 8);
        m_state[2][1] = (UINT8)(value >> 16);
        m_state[3][1] = (UINT8)(value >> 24);

        value = mixColumn_a[r_state[0][2]] ^
                mixColumn_b[r_state[1][2]] ^
                mixColumn_c[r_state[2][2]] ^
                mixColumn_d[r_state[3][2]] ^
                r_key[2];
        m_state[0][2] = (UINT8)value;
        m_state[1][2] = (UINT8)(value >> 8);
        m_state[2][2] = (UINT8)(value >> 16);
        m_state[3][2] = (UINT8)(value >> 24);

        value = mixColumn_a[r_state[0][3]] ^
                mixColumn_b[r_state[1][3]] ^
                mixColumn_c[r_state[2][3]] ^
                mixColumn_d[r_state[3][3]] ^
                r_key[3];
        m_state[0][3] = (UINT8)value;
        m_state[1][3] = (UINT8)(value >> 8);
        m_state[2][3] = (UINT8)(value >> 16);
        m_state[3][3] = (UINT8)(value >> 24);

        r_key += 4;
    }while (--round);

    // The last round is special because does not have MixColumns
    // Make SubBytes, ShiftRows and AddRoundKey in the same step and copy the result in destination
    dst32[0] = ( sbox[m_state[0][0]]        |
                (sbox[m_state[1][1]] << 8)  |
                (sbox[m_state[2][2]] << 16) |
                (sbox[m_state[3][3]] << 24)
               ) ^ r_key[0];

    dst32[1] = ( sbox[m_state[0][1]]        |
                (sbox[m_state[1][2]] << 8)  |
                (sbox[m_state[2][3]] << 16) |
                (sbox[m_state[3][0]] << 24)
               ) ^ r_key[1];

    dst32[2] = ( sbox[m_state[0][2]]        |
                (sbox[m_state[1][3]] << 8)  |
                (sbox[m_state[2][0]] << 16) |
                (sbox[m_state[3][1]] << 24)
               ) ^ r_key[2];

    dst32[3] = ( sbox[m_state[0][3]]        |
                (sbox[m_state[1][0]] << 8)  |
                (sbox[m_state[2][1]] << 16) |
                (sbox[m_state[3][2]] << 24)
               ) ^ r_key[3];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_CPU_Impl_O1::DecryptBlock(UINT8 *dst, const UINT8 *src)
{
    UINT32 *r_key = (UINT32*)m_keyDec,
           *src32 = (UINT32*)src,
           *dst32 = (UINT32*)dst;
    UINT8 m_state[4][4];
    UINT32 value;

    // First round will copy the data in the state and add the round key
    value = src32[0] ^ r_key[0];
    m_state[0][0] = (UINT8)value;
    m_state[1][0] = (UINT8)(value >> 8);
    m_state[2][0] = (UINT8)(value >> 16);
    m_state[3][0] = (UINT8)(value >> 24);

    value = src32[1] ^ r_key[1];
    m_state[0][1] = (UINT8)value;
    m_state[1][1] = (UINT8)(value >> 8);
    m_state[2][1] = (UINT8)(value >> 16);
    m_state[3][1] = (UINT8)(value >> 24);

    value = src32[2] ^ r_key[2];
    m_state[0][2] = (UINT8)value;
    m_state[1][2] = (UINT8)(value >> 8);
    m_state[2][2] = (UINT8)(value >> 16);
    m_state[3][2] = (UINT8)(value >> 24);

    value = src32[3] ^ r_key[3];
    m_state[0][3] = (UINT8)value;
    m_state[1][3] = (UINT8)(value >> 8);
    m_state[2][3] = (UINT8)(value >> 16);
    m_state[3][3] = (UINT8)(value >> 24);

    // Do the rest of the rounds
    r_key += 4;
    UINT32 round = m_Nr - 1;
    do
    {
        UINT8 r_state[4][4];
        // Make InvShiftRows and InvSubBytes in the same step and put the result in r_state
        r_state[0][0] = rsbox[m_state[0][0]];
        r_state[0][1] = rsbox[m_state[0][1]];
        r_state[0][2] = rsbox[m_state[0][2]];
        r_state[0][3] = rsbox[m_state[0][3]];

        r_state[1][0] = rsbox[m_state[1][3]];
        r_state[1][1] = rsbox[m_state[1][0]];
        r_state[1][2] = rsbox[m_state[1][1]];
        r_state[1][3] = rsbox[m_state[1][2]];

        r_state[2][0] = rsbox[m_state[2][2]];
        r_state[2][1] = rsbox[m_state[2][3]];
        r_state[2][2] = rsbox[m_state[2][0]];
        r_state[2][3] = rsbox[m_state[2][1]];

        r_state[3][0] = rsbox[m_state[3][1]];
        r_state[3][1] = rsbox[m_state[3][2]];
        r_state[3][2] = rsbox[m_state[3][3]];
        r_state[3][3] = rsbox[m_state[3][0]];

        // Make InvMixColumns and AddRoundKey in the same step and put the result back in m_state
        value = invMixColumn_a[r_state[0][0]] ^
                invMixColumn_b[r_state[1][0]] ^
                invMixColumn_c[r_state[2][0]] ^
                invMixColumn_d[r_state[3][0]] ^
                r_key[0];
        m_state[0][0] = (UINT8)value;
        m_state[1][0] = (UINT8)(value >> 8);
        m_state[2][0] = (UINT8)(value >> 16);
        m_state[3][0] = (UINT8)(value >> 24);

        value = invMixColumn_a[r_state[0][1]] ^
                invMixColumn_b[r_state[1][1]] ^
                invMixColumn_c[r_state[2][1]] ^
                invMixColumn_d[r_state[3][1]] ^
                r_key[1];
        m_state[0][1] = (UINT8)value;
        m_state[1][1] = (UINT8)(value >> 8);
        m_state[2][1] = (UINT8)(value >> 16);
        m_state[3][1] = (UINT8)(value >> 24);

        value = invMixColumn_a[r_state[0][2]] ^
                invMixColumn_b[r_state[1][2]] ^
                invMixColumn_c[r_state[2][2]] ^
                invMixColumn_d[r_state[3][2]] ^
                r_key[2];
        m_state[0][2] = (UINT8)value;
        m_state[1][2] = (UINT8)(value >> 8);
        m_state[2][2] = (UINT8)(value >> 16);
        m_state[3][2] = (UINT8)(value >> 24);

        value = invMixColumn_a[r_state[0][3]] ^
                invMixColumn_b[r_state[1][3]] ^
                invMixColumn_c[r_state[2][3]] ^
                invMixColumn_d[r_state[3][3]] ^
                r_key[3];
        m_state[0][3] = (UINT8)value;
        m_state[1][3] = (UINT8)(value >> 8);
        m_state[2][3] = (UINT8)(value >> 16);
        m_state[3][3] = (UINT8)(value >> 24);

        r_key += 4;
    }while(--round);

    // The last round is special because does not have InvMixColumns
    // Make InvShiftRows, InvSubBytes and AddRoundKey in the same step and copy the result in destination
    dst32[0] = ( rsbox[m_state[0][0]]       |
                (rsbox[m_state[1][3]] << 8) |
                (rsbox[m_state[2][2]] << 16)|
                (rsbox[m_state[3][1]] << 24)
               ) ^ r_key[0];

    dst32[1] = ( rsbox[m_state[0][1]]       |
                (rsbox[m_state[1][0]] << 8) |
                (rsbox[m_state[2][3]] << 16)|
                (rsbox[m_state[3][2]] << 24)
               ) ^ r_key[1];

    dst32[2] = ( rsbox[m_state[0][2]]       |
                (rsbox[m_state[1][1]] << 8) |
                (rsbox[m_state[2][0]] << 16)|
                (rsbox[m_state[3][3]] << 24)
               ) ^ r_key[2];

    dst32[3] = ( rsbox[m_state[0][3]]       |
                (rsbox[m_state[1][2]] << 8) |
                (rsbox[m_state[2][1]] << 16)|
                (rsbox[m_state[3][0]] << 24)
               ) ^ r_key[3];
}
