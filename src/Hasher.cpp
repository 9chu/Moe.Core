/**
 * @file
 * @date 2017/7/5
 * @see https://github.com/WaterJuice/CryptLib
 */
#include <Moe.Core/Hasher.hpp>

#include <cstring>

using namespace std;
using namespace moe;
using namespace Hasher;

//////////////////////////////////////////////////////////////////////////////// details

static const uint32_t kMpqCryptTable[0x500] = {
    0x55C636E2u, 0x02BE0170u, 0x584B71D4u, 0x2984F00Eu, 0xB682C809u, 0x91CF876Bu, 0x775A9C24u, 0x597D5CA5u,
    0x5A1AFEB2u, 0xD3E9CE0Du, 0x32CDCDF8u, 0xB18201CDu, 0x3CCE05CEu, 0xA55D13BEu, 0xBB0AFE71u, 0x9376AB33u,
    0x848F645Eu, 0x87E45A45u, 0x45B86017u, 0x5E656CA8u, 0x1B851A95u, 0x2542DBD7u, 0xAB4DF9E4u, 0x5976AE9Bu,
    0x6C317E7Du, 0xCDDD2F94u, 0x3C3C13E5u, 0x335B1371u, 0x31A592CAu, 0x51E4FC4Cu, 0xF7DB5B2Fu, 0x8ABDBE41u,
    0x8BEAA674u, 0x20D6B319u, 0xDE6C9A9Du, 0xC5AC84E5u, 0x445A5FEBu, 0x94958CB0u, 0x1E7D3847u, 0xF35D29B0u,
    0xCA5CCEDAu, 0xB732C8B5u, 0xFDCC41DDu, 0x0EDCEC16u, 0x9D01FEAEu, 0x1165D38Eu, 0x9EE193C8u, 0xBF33B13Cu,
    0x61BC0DFCu, 0xEF3E7BE9u, 0xF8D4D4C5u, 0xC79B7694u, 0x5A255943u, 0x0B3DD20Au, 0x9D1AB5A3u, 0xCFA8BA57u,
    0x5E6D7069u, 0xCB89B731u, 0x3DC0D15Bu, 0x0D4D7E7Eu, 0x97E37F2Bu, 0xFEFC2BB1u, 0xF95B16B5u, 0x27A55B93u,
    0x45F22729u, 0x4C986630u, 0x7C666862u, 0x5FA40847u, 0xA3F16205u, 0x791B7764u, 0x386B36D6u, 0x6E6C3FEFu,
    0xC75855DBu, 0x4ABC7DC7u, 0x4A328F9Bu, 0xCEF20C0Fu, 0x60B88F07u, 0xF7BB4B8Fu, 0x830B5192u, 0x94F711ECu,
    0x20250752u, 0x399D21A3u, 0xE5C0840Du, 0xE76CFFA5u, 0x624FAB29u, 0x5DF133E6u, 0x83E0B9B8u, 0xC5796BFBu,
    0x4A7AB2D0u, 0xBA59A821u, 0x03A81E4Cu, 0xCD3ADFDBu, 0x32B26B8Cu, 0x8E35C533u, 0x9E6300E9u, 0x8CF92AC5u,
    0x880D18EBu, 0x131A53B3u, 0x2ED2DC64u, 0xB23257C1u, 0xA06450C1u, 0x1B92CB8Eu, 0x72ED730Eu, 0x19A685F0u,
    0x82836483u, 0x42D94E8Au, 0xEE9BD6F6u, 0x556D0B6Au, 0xBA65589Au, 0xDE24CCE4u, 0x53329F6Cu, 0xC754FE8Bu,
    0x503D2DC7u, 0x10027BA4u, 0xD3B60A8Bu, 0x68E68D83u, 0x0A9128A9u, 0x595FA35Fu, 0x0B03B5BEu, 0x150A45C4u,
    0xB1629CCEu, 0xE5F7497Bu, 0x8A7098A4u, 0xB8233E69u, 0x8EA0F978u, 0x5B579970u, 0xEAB14318u, 0x4B28B263u,
    0xB6766CEFu, 0x06782877u, 0x155C6DD0u, 0xC711333Cu, 0xF819CEDFu, 0x00EB1D68u, 0xD6FFFA6Eu, 0x439E5962u,
    0xD765D6DBu, 0xCB0BCEE9u, 0x6D3C5647u, 0x965466F3u, 0x0CA983C9u, 0x74ECC1CEu, 0xFC0563B6u, 0x42B08FEEu,
    0xC5B38853u, 0xFE502CEBu, 0x7B432FAFu, 0xC309E610u, 0x2C3997D8u, 0x43774654u, 0x15BD9D2Cu, 0xED6A420Du,
    0xC7FF520Cu, 0xB8A97FD1u, 0x5E4D60CCu, 0xB9738D11u, 0xDA2181FFu, 0x73AC2597u, 0x3A8EEC8Du, 0xAC85E779u,
    0xF3F975D6u, 0xB9FE7B91u, 0x0F155D1Eu, 0x2860B6DDu, 0x835977CBu, 0xB0607436u, 0x9CAB7F6Bu, 0x8AB91186u,
    0xC12B51E9u, 0x20084E8Bu, 0x44BA8EADu, 0xA542B130u, 0x82BCD5C4u, 0xCC747F4Eu, 0x0F1909D8u, 0xDA242E1Cu,
    0x6F7D1AA0u, 0xD2626486u, 0x88D0781Eu, 0xAB695CCDu, 0xFA569145u, 0xB4FEB55Cu, 0xBE47E896u, 0xE70A7A88u,
    0xD56185A2u, 0xACF4C871u, 0x09282332u, 0x1DDEEAA8u, 0x590C7ADBu, 0xF4A97667u, 0xBFD85705u, 0x0EA77CCCu,
    0xA9F85364u, 0x83195869u, 0x8BFB041Au, 0xDB842F5Cu, 0xD6F0F315u, 0xA7756EA7u, 0x0A51B439u, 0xA9EDF8A3u,
    0xD9084E2Fu, 0x827407F8u, 0xD4AC8284u, 0x09739D0Du, 0xB3BB6CFCu, 0xD539C77Du, 0x6BBC9AC0u, 0x35C641AAu,
    0x934C96B0u, 0xD17AF317u, 0x29C6BAEFu, 0xB275CDACu, 0xD72662DEu, 0x9F5C2544u, 0xC1A98F75u, 0xD98E8F9Au,
    0x47BD5C86u, 0x70C610A6u, 0xB5482ED4u, 0x23B9C68Cu, 0x3C1BAE66u, 0x69556E7Fu, 0xD902F5E0u, 0x653D195Bu,
    0xDE6541FBu, 0x07BCC6ACu, 0xC6EE7788u, 0x801534D4u, 0x2C1F35C0u, 0xD9DE614Du, 0xBDCCAC85u, 0xB4D4A0DAu,
    0x242D549Bu, 0x9D964796u, 0xB9CEB982u, 0x59FA99A9u, 0xD8986CC1u, 0x9E90C1A1u, 0x01BBD82Fu, 0xD7F1C5FDu,
    0xDD847EBAu, 0x883D305Du, 0x25F13152u, 0x4A92694Du, 0x77F1E601u, 0x8024E6E7u, 0x02A5F53Du, 0x9C3EF4D9u,
    0xAF403CCCu, 0xE2AD03C0u, 0x46EDF6ECu, 0x6F9BD3E6u, 0xCC24AD7Au, 0x47AFAB12u, 0x82298DF7u, 0x708C9EECu,
    0x76F8C1B1u, 0xB39459D2u, 0x3F1E26D9u, 0xE1811BE7u, 0x56ED1C4Du, 0xC9D18AF8u, 0xE828060Eu, 0x91CADA2Eu,
    0x5CCBF9B7u, 0xF1A552D4u, 0x3C9D4343u, 0xE1008785u, 0x2ADFEEBFu, 0xF90240A0u, 0x3D08CCE7u, 0x426E6FB0u,
    0x573C984Fu, 0x13A843AEu, 0x406B7439u, 0x636085D9u, 0x5000BA9Au, 0xAD4A47ABu, 0xAF001D8Du, 0x419907AEu,
    0x185C8F96u, 0xE5E9ED4Du, 0x61764133u, 0xD3703D97u, 0xAC98F0C6u, 0xDBC3A37Cu, 0x85F010C4u, 0x90491E32u,
    0xF12E18BFu, 0xC88C96E1u, 0xD3FBD6D9u, 0xE3C28B08u, 0xD5BF08CCu, 0xB1E78859u, 0x2546DDCFu, 0xB030B200u,
    0xAAFD2811u, 0x55B22D21u, 0xD38BF567u, 0x469C7A2Bu, 0x5AD05792u, 0xA1A5981Eu, 0x7DFB8384u, 0x34D1CA0Au,
    0x7EB0DBE0u, 0xD61CE0F6u, 0x398068B7u, 0xE6406D1Fu, 0x95AE6B47u, 0xE4281230u, 0xB0843061u, 0xA70A3A68u,
    0xE340F625u, 0x72DCBFFDu, 0x8EB8AFCDu, 0x18B6661Fu, 0x17EF5A5Cu, 0x000C5B22u, 0x6BA13836u, 0x6165E383u,
    0x74481C5Bu, 0xE56F0711u, 0xA26F5024u, 0x5FF22E60u, 0x31A5E829u, 0xA1094BF0u, 0xC680EC6Cu, 0x8CF327D7u,
    0xEBF1348Au, 0x6A227D2Fu, 0x74065184u, 0x8DF65112u, 0x2BBD05EEu, 0xE4D00ED6u, 0x2980EE1Au, 0x6AE1DA73u,
    0xE84614DAu, 0x6C9906ABu, 0xCF8E02DBu, 0xD3723E97u, 0x92F66CAFu, 0xAC8491C7u, 0xAEC65696u, 0xB98997CFu,
    0xFA16C762u, 0x6D73C65Fu, 0x205D22A6u, 0x4DD3AAA5u, 0x2DEB6BC0u, 0x9F37686Cu, 0x71A5282Bu, 0x376BB9E0u,
    0x7FFF2A1Bu, 0xDE67982Fu, 0x9CBF33CEu, 0x2E6DAB37u, 0x6E3424B9u, 0x0EE143BCu, 0x832A60D9u, 0xBB6329E1u,
    0x13F6BEFDu, 0x5965FB84u, 0xF60B233Cu, 0x3D695183u, 0x433224A1u, 0xB5D9CAE5u, 0x82459BABu, 0x9F21B311u,
    0xAF6C5247u, 0xB447B13Au, 0x7B2676C3u, 0xC38979CDu, 0x8526AE25u, 0xC550AD5Bu, 0x685099A7u, 0x65E9C2BDu,
    0xE5C6DC36u, 0xE10B37A9u, 0x88016878u, 0xCE81D4E4u, 0x24D6FC80u, 0x4106152Du, 0x6D4F5F90u, 0xC4DC74BEu,
    0xDB48676Cu, 0x6CB569B7u, 0xF3BF598Fu, 0x042B08D9u, 0x02CCB2DEu, 0xB1056F65u, 0x47994AF4u, 0xFA141BA4u,
    0x9376AB2Eu, 0x07A76737u, 0x75E7E6FCu, 0x449D80A1u, 0x03B7259Du, 0xF6DF358Au, 0x5A75D5B9u, 0x47286923u,
    0x3B1A30EFu, 0xEEBE3D6Au, 0x9DB1AA00u, 0x007A90D9u, 0x24667071u, 0x019C73CFu, 0x69039BCDu, 0x95900744u,
    0x6518B1EBu, 0x6905F202u, 0xEE3951B2u, 0xE141FCA9u, 0x797FA832u, 0x5A95E55Bu, 0xD6263B15u, 0x5B61F394u,
    0x897ACB1Cu, 0x005F83A9u, 0x22420F71u, 0xF495176Eu, 0x7E138F3Du, 0x1392E384u, 0x373BF7AAu, 0x8E512816u,
    0xA960B3CAu, 0x0474D74Cu, 0xFFACD6D7u, 0x2EF5ED9Eu, 0x60992AAAu, 0x7E690E99u, 0x23C0749Du, 0xD8E29105u,
    0x555D5909u, 0x15631BFEu, 0xA69C5A1Cu, 0x501017CAu, 0x99438048u, 0x38733AC7u, 0xE682E2C8u, 0xD4655FD6u,
    0x956E4C04u, 0x347DF643u, 0x2F4B177Bu, 0x93ED3AA4u, 0xA77E1DD5u, 0x7AE55702u, 0xD2A52FD9u, 0xEF8BA18Cu,
    0xB7D3C1EEu, 0x8078BA8Du, 0xAB5AAADBu, 0x752BE08Fu, 0x068B31C1u, 0x078AAE3Cu, 0xAA5A8343u, 0x123D9268u,
    0x2CEAEE43u, 0x8EBDB239u, 0x650251F3u, 0x04883648u, 0x8C62E12Eu, 0x12B32167u, 0xE5112E9Au, 0x10002548u,
    0x3E7A818Du, 0x077E5327u, 0xF140CC21u, 0x6CE7D75Du, 0x9B99F9A5u, 0x3215741Cu, 0xB6AADBAEu, 0x738768DCu,
    0x82A3742Fu, 0x76517020u, 0xDD872AD8u, 0x9D0902B2u, 0x7D1A6B04u, 0x49381592u, 0x63A652A5u, 0x0C15E626u,
    0xE22F70D6u, 0x01E84385u, 0xB29DE134u, 0x20C5000Eu, 0xE961F443u, 0x2D31662Eu, 0x3CE6BC28u, 0x34F9DD94u,
    0xFA45DE53u, 0x497588BDu, 0x9468215Bu, 0x0777FA5Cu, 0x6F7114C0u, 0xE0E82694u, 0xE4371986u, 0x57112DE2u,
    0xE0CAC289u, 0xF2A3CEE0u, 0x6A41E1B9u, 0xBFCEA77Du, 0xF927FD52u, 0x69747D98u, 0xBEA76CDBu, 0x8DD39557u,
    0x04DB5ECEu, 0x2A0885C8u, 0x3BE4E8EEu, 0x21D785DCu, 0x09DE7C0Eu, 0x3258EA33u, 0x51922982u, 0xEE8DD024u,
    0x3DF6965Du, 0x30C1237Bu, 0xF7F6686Au, 0x9FACA186u, 0x7C400076u, 0x85ACEF8Au, 0xF4B6D220u, 0xDDC3481Cu,
    0x439EAEC4u, 0x717BBE63u, 0x8259FAA7u, 0xD682BD68u, 0x932A8610u, 0x38BF0A7Fu, 0x6212E2C7u, 0x88EE3168u,
    0xB3C27047u, 0x6133CB1Eu, 0x15295506u, 0x5AE66246u, 0x1D208DDDu, 0xA91D3DBAu, 0xC315968Du, 0x6AA2664Bu,
    0x716D0CCAu, 0x891F4956u, 0x80866BFFu, 0xBD56C847u, 0x9093425Au, 0x28DD9E87u, 0x84EF3E08u, 0x690A49D6u,
    0x6A7EFF82u, 0xABCFE400u, 0x3D3BE5CAu, 0x381B650Cu, 0x4B7C8622u, 0x3E0246F3u, 0xA3561654u, 0x9488865Cu,
    0x3AEF1BF2u, 0x5E5D68A2u, 0xD32F1DDCu, 0x51972BF0u, 0x177A213Bu, 0x469375C2u, 0x37640BD0u, 0xFC3324C8u,
    0x07091A09u, 0x2D63D3FBu, 0x2153F023u, 0x48223875u, 0x61A55826u, 0x8C136538u, 0x49F71D98u, 0x84C7D51Eu,
    0x85551A73u, 0x13D604C5u, 0xD701A626u, 0x87B844CAu, 0x741EB29Du, 0x2A2C977Cu, 0xC797CA03u, 0x6C4085D7u,
    0x2DACF79Bu, 0x734FA2EBu, 0xCC290557u, 0xFA1E75E4u, 0x06B29A27u, 0xBECE2A7Au, 0x70A4554Bu, 0xC935942Eu,
    0xA764BBC1u, 0x1FE391D6u, 0x7807F0C2u, 0x40606ED9u, 0xE5153086u, 0xE91D7DD2u, 0xED5D3BA9u, 0xAA14B64Au,
    0x83B24DD9u, 0xEC1FF5CDu, 0xBA33EAD3u, 0xE4EF735Cu, 0xBC062438u, 0xD8BFD523u, 0x473D1E04u, 0x2007F8A7u,
    0xB02903EDu, 0x86EA8ADAu, 0x95AB69CFu, 0xFD1F9809u, 0x9CB3D8BBu, 0x51F45958u, 0x9CDD4276u, 0xC245865Eu,
    0x8F0C836Bu, 0x4EE7DC07u, 0xF6368D9Du, 0xEF2C1DC1u, 0xEE56B54Bu, 0xBD62CE2Fu, 0xF4916AADu, 0xC81CB594u,
    0x41729F49u, 0x24BEF0A4u, 0xDEF487A9u, 0x222E05B8u, 0x8D3BF5C6u, 0x11B55009u, 0xAD09D2B3u, 0x19DB9FD1u,
    0xD7427085u, 0x33DBFC8Bu, 0x526B9378u, 0x790E1BC8u, 0xB2998A00u, 0xA5641703u, 0x0676D249u, 0x6B9185CCu,
    0x30E4348Fu, 0x82C52F65u, 0x57C7DC24u, 0x489C1ECDu, 0x9FCAB02Au, 0x56D61117u, 0xFE869CACu, 0x55FC5140u,
    0x7FBBB382u, 0x9E5AFC79u, 0x10047C99u, 0xFC9F5984u, 0x56587E2Du, 0xB98193F0u, 0x98FE5E8Eu, 0x29B15B6Bu,
    0x9561F055u, 0xBB0CAA25u, 0x1E4ECC15u, 0x23F5393Bu, 0x0845B458u, 0xCEFF67CAu, 0xB099900Cu, 0x00B1564Fu,
    0x39EEF3D1u, 0xFCC1BF84u, 0xAC8893B5u, 0x6484BF0Eu, 0x91C02AB3u, 0x8C0C0C70u, 0x686FA8C6u, 0xE171BED6u,
    0xDFAE37DFu, 0xD5A1A4E7u, 0xE3EB49A1u, 0x5E6014E0u, 0x205B21ACu, 0xFD58B3DAu, 0x2E7C07CDu, 0xEF2CC85Au,
    0xD7587B46u, 0xF417847Du, 0x8A30CEC1u, 0x70984F6Cu, 0xF0B63388u, 0xC220C98Du, 0xEDE62936u, 0x92C0A7B3u,
    0x1EF371E8u, 0x2005F7AFu, 0x91A47265u, 0xB0CF5504u, 0xD500ABA8u, 0xCB5C4BD3u, 0x9B3BCBC3u, 0xCF6644B5u,
    0xCE9488EFu, 0x003FC96Eu, 0xAA42222Fu, 0x4844F3D0u, 0x4DB89D77u, 0x08681AAEu, 0x662F3A28u, 0x761552DBu,
    0x1DF7A17Au, 0x93FEED9Au, 0xCC496A4Fu, 0xA217CFCDu, 0x3BA3C930u, 0x268F7E77u, 0x0797B4A1u, 0x8BEBFC51u,
    0x068930C4u, 0x16C874E2u, 0xC242DA24u, 0xFB229F76u, 0xA0795B02u, 0x689FC036u, 0x17A73732u, 0xD21AEC00u,
    0xAC00A692u, 0x5B217F18u, 0xAE421624u, 0x2BC05CC0u, 0x48C1DB7Au, 0x4F4E63B4u, 0x1667F04Eu, 0x34020F94u,
    0x972B2555u, 0x9A07355Bu, 0x01665970u, 0x7DB60C6Fu, 0x3AD7103Bu, 0x5C3D09C0u, 0xEEA3DADAu, 0x88C21C10u,
    0x102436D7u, 0x6A3B3400u, 0xEB523C4Cu, 0xFB97D896u, 0x964CB86Bu, 0xDD878038u, 0x0529DA4Du, 0x0B1468A5u,
    0x18739AC8u, 0xF7F26668u, 0xF64F4471u, 0x5C14F5C3u, 0x44A081FBu, 0x39AC7E37u, 0x8A17C26Bu, 0x868F5E67u,
    0x3931978Du, 0x6EDF7817u, 0x4951CC67u, 0x943407F3u, 0xCC5E748Fu, 0x2B7EE729u, 0xCBB320F0u, 0x11FEC8E7u,
    0xFCCFC658u, 0x03454354u, 0x373AA1ECu, 0x1D58FE9Au, 0x064710AEu, 0xA88AA0BAu, 0xD183A23Eu, 0x40D150A3u,
    0xF531B8D1u, 0xA7D99F85u, 0x11838CD5u, 0xB19E64B3u, 0x3D67A5E9u, 0xB02C5AC6u, 0x99B9B9E8u, 0x4C202B7Au,
    0x15F261D3u, 0xA84C2D0Du, 0x50F185A6u, 0x33BA41D5u, 0x39791013u, 0x4BAFF44Eu, 0xEEEEAA1Cu, 0xE0488314u,
    0x559CCD2Bu, 0xA104F445u, 0x636F37C4u, 0x264D5E3Bu, 0x75C17F35u, 0x75424131u, 0xBB115739u, 0x74FE755Au,
    0x7D3A7AA6u, 0x2D8BE784u, 0x83ED154Au, 0xFC2673D8u, 0x44DD4A7Fu, 0x79056CC8u, 0x82CC8831u, 0x9D3C1B7Cu,
    0xE9453BFAu, 0x24315694u, 0x661F3253u, 0x75549F5Cu, 0xBB2B63EDu, 0x67E00D96u, 0xF48966C7u, 0x0D7BEA56u,
    0xC25F92EFu, 0xA947A79Du, 0xDE4ADF6Fu, 0xAC0F0342u, 0xD3EB246Bu, 0xA4AA118Eu, 0x3C3E6A46u, 0x457F4441u,
    0xA50A406Fu, 0x6C508D9Fu, 0xE9AC18E7u, 0x1ECDB4BAu, 0x39AC7E3Au, 0x7FB304FAu, 0x6F38F8E8u, 0x4AECEA6Du,
    0x61035E73u, 0x81708907u, 0xEBC07205u, 0x90FD7614u, 0xB52D217Fu, 0x6C4DE195u, 0x1DD49084u, 0x64EE482Cu,
    0x94C7A521u, 0x540C09D8u, 0x75DF8DD5u, 0x414131F7u, 0x3698FD76u, 0xF784DB4Fu, 0xF8C97A03u, 0x048F39B9u,
    0x3BF4F0BDu, 0x8CB50992u, 0x9B58D9EEu, 0xE5AB79CCu, 0x9A5F6052u, 0xBD9591B0u, 0xFAD2232Bu, 0x5A632254u,
    0x0286E618u, 0x8AD3C8F7u, 0xE4060176u, 0x754C4617u, 0x5C10490Bu, 0x6F7D6FFFu, 0x2187B42Au, 0x5775095Bu,
    0x02F4C663u, 0x5A5DCA06u, 0xFE4AD4C7u, 0x53E19F7Du, 0x59FF46B5u, 0xBCC42BA5u, 0xFD2F4A97u, 0xBED6D905u,
    0x95629B6Bu, 0x21A1C0DBu, 0xAA10B45Du, 0xE6EF6D58u, 0x2892CF4Du, 0x9FED6C10u, 0x1E386BF7u, 0x9BE0C6E8u,
    0x2B2F15EFu, 0x19F5AC7Bu, 0x7AFF0E72u, 0x31DA576Fu, 0x30252CB4u, 0x577960ACu, 0x166E9E5Au, 0xA9374A61u,
    0x71369C96u, 0x7FF826AEu, 0xE8175326u, 0xCABBFD33u, 0x0191190Eu, 0x699D3C3Eu, 0x36B40B22u, 0xB3950513u,
    0x9B889BFAu, 0xA52A5007u, 0xAC290FEDu, 0x3B4E4A4Fu, 0xB753D8D6u, 0x3C531F22u, 0x582F6427u, 0xA9CD93A9u,
    0x546E39AEu, 0x242FAAD2u, 0xD2E0F747u, 0x09F6325Du, 0x59D48719u, 0xAD7EB66Eu, 0xD5512878u, 0x56DEBF9Du,
    0x5107E5A5u, 0xF1C00AA4u, 0x814CCCA8u, 0x600D90F0u, 0x9BE97619u, 0x915FA5F2u, 0x2B5628DDu, 0xA33D5F5Au,
    0x595DF7C1u, 0x6966215Du, 0x50EC8337u, 0xF1D21372u, 0x0EE2EEFBu, 0xAD9E70B7u, 0xAB0D2FE4u, 0xCF277B5Du,
    0x62585A2Cu, 0x835A7844u, 0x74B1FA6Bu, 0x49BAFFD5u, 0x2EA9C864u, 0x129311A8u, 0xBDFA1867u, 0x83CA5997u,
    0x9D1DB719u, 0x84BB79E6u, 0x9E3F99F2u, 0x313F6101u, 0x1B99245Bu, 0xD15D8FB2u, 0xCEF90F81u, 0x2945268Du,
    0xDBBCF573u, 0xB1021886u, 0x9EE7EC1Du, 0x1CF824F7u, 0x7EAA2E32u, 0x69C0A2B5u, 0x7494419Cu, 0xE253D7D3u,
    0x48DA3D12u, 0x45B8B571u, 0xDB4D147Au, 0xD82D8DDEu, 0x265D10A2u, 0xB0A6EB9Au, 0x7E1C93A6u, 0x36FE2F46u,
    0xDCAD6B00u, 0x05439191u, 0xB0CE5484u, 0x61D1C309u, 0x8DA62A03u, 0x06D0FE2Fu, 0xBAC6DD3Cu, 0xCA2006F3u,
    0x8321B1AFu, 0x0411A6F3u, 0xE8918EACu, 0x21A2C152u, 0x91C0D54Fu, 0x6AAA14FAu, 0xDD22A440u, 0x88CB2075u,
    0x7A4EB813u, 0x67AFA071u, 0xD8D98C9Cu, 0x31F10D47u, 0x6FF1A8A8u, 0x2FAAF0A1u, 0x48A221BBu, 0x3BE6948Bu,
    0xAA79E79Bu, 0x0EA7278Cu, 0x7A3857EFu, 0x49B7FE55u, 0xD51CB931u, 0x041C018Du, 0x00B90501u, 0x45EA7881u,
    0x8FC1DBCFu, 0xB80B32A9u, 0xABACD2E9u, 0x677BDC40u, 0xECACE542u, 0x6D6514EBu, 0x31C09FF7u, 0x5E6C1ABDu,
    0x1C391D0Fu, 0x0E9D77F1u, 0x7119392Du, 0x6BE9B0BAu, 0x6194FA77u, 0x45E62148u, 0x42234AF2u, 0xC3239D66u,
    0x939CBDBCu, 0x56200D9Cu, 0x6B275208u, 0x001A61F3u, 0xCCC2A546u, 0x4B722BE0u, 0xEE25F2B7u, 0x6D86CF9Eu,
    0xAA6BE0CDu, 0x4DCDA7B6u, 0x78D4AA13u, 0x36EA7AD9u, 0x3F29D700u, 0xDEEA2D84u, 0x6A6AF5BDu, 0x18AFB81Cu,
    0xD8E4E73Cu, 0x8AA708BAu, 0x658B94D9u, 0xA676478Cu, 0xCFA10C22u, 0x25593C74u, 0x8D962235u, 0x5F980270u,
    0x3DF6EBC0u, 0x8E7D92FAu, 0xC3EE55E1u, 0xD5F72447u, 0x02B0FA95u, 0x52B0B520u, 0x70D2C11Fu, 0x3A6FDD6Cu,
    0x193AA698u, 0x5496F7D5u, 0x4208931Bu, 0x7A4106ECu, 0x83E86840u, 0xF49B6F8Cu, 0xBA3D9A51u, 0x55F54DDDu,
    0x2DE51372u, 0x9AFB571Bu, 0x3AB35406u, 0xAD64FF1Fu, 0xC77764FEu, 0x7F864466u, 0x416D9CD4u, 0xA2489278u,
    0xE30B86E4u, 0x0B5231B6u, 0xBA67AED6u, 0xE5AB2467u, 0x60028B90u, 0x1D9E20C6u, 0x2A7C692Au, 0x6B691CDBu,
    0x9E51F817u, 0x9B763DECu, 0x3D29323Fu, 0xCFE12B68u, 0x754B459Bu, 0xA2238047u, 0xD9C55514u, 0x6BDCFFC1u,
    0x693E6340u, 0x82383FE7u, 0x1916EA5Fu, 0xEC7BCD59u, 0x72DE165Au, 0xE79A1617u, 0x8EC86234u, 0xA8F0D284u,
    0x20C90226u, 0x7BF98884u, 0x28A58331u, 0x3EC3FA6Eu, 0x4CE0895Bu, 0xC353B4D0u, 0x33EF064Fu, 0x21E5E210u,
    0xC8BB589Du, 0xE85DCAB2u, 0xAC65829Fu, 0xA7BF92D0u, 0x05A6174Du, 0x25A50C2Eu, 0xE5C78777u, 0x3D75021Fu,
    0x4BAA9C98u, 0x23BDC884u, 0x9653BBD7u, 0xBADCE7F5u, 0xC283A484u, 0xC040DF2Eu, 0x9370A841u, 0x2F316022u,
    0x36EED231u, 0xAC2CBC0Cu, 0x13C0A49Bu, 0xCDD12997u, 0x07FE91B2u, 0xCD7EABCDu, 0x2C01271Du, 0x18432DF8u,
    0x599C6BC7u, 0x75E93D5Au, 0xB67A6EE2u, 0x8E738E16u, 0xFF9073FDu, 0xAF77026Au, 0xF86EA2FCu, 0x91509EA3u,
    0x33A78DC6u, 0x4F79234Au, 0x3A7535BCu, 0x3539FCB1u, 0x3103EE52u, 0x4F6F1E69u, 0x6BB3EBBCu, 0x4CB77555u,
    0x8DD1E999u, 0x2ADE439Du, 0x11521FAEu, 0xB94D2545u, 0x8DDE9ABDu, 0x1909393Fu, 0xB792A23Du, 0x749C455Bu,
    0xB5B60F2Cu, 0x380459CEu, 0x0DAD5820u, 0xB130845Bu, 0x291CBD52u, 0xDE9A5BB7u, 0x51DEF961u, 0x515B6408u,
    0xCA6E823Eu, 0x382E6E74u, 0xEEBE3D71u, 0x4C8F0C6Au, 0xE676DCEAu, 0x14E1DC7Cu, 0x6F7FC634u, 0xCF85A943u,
    0xD39EA96Eu, 0x136E7C93u, 0x7164B304u, 0xF32F1333u, 0x35C34034u, 0xDE39D721u, 0x91A87439u, 0xC410111Fu,
    0x29F17AACu, 0x1316A6FFu, 0x12F194EEu, 0x420B9499u, 0xF72DB0DCu, 0x690B9F93u, 0x17D14BB2u, 0x8F931AB8u,
    0x217500BCu, 0x875413F8u, 0x98B2E43Du, 0xC51F9571u, 0x54CEBDCAu, 0x0719CC79u, 0xF3C7080Du, 0xE4286771u,
    0xA3EAB3CDu, 0x4A6B00E0u, 0x11CF0759u, 0x7E897379u, 0x5B32876Cu, 0x5E8CD4F6u, 0x0CEDFA64u, 0x919AC2C7u,
    0xB214F3B3u, 0x0E89C38Cu, 0xF0C43A39u, 0xEAE10522u, 0x835BCE06u, 0x9EEC43C2u, 0xEA26A9D6u, 0x69531821u,
    0x6725B24Au, 0xDA81B0E2u, 0xD5B4AE33u, 0x080F99FBu, 0x15A83DAFu, 0x29DFC720u, 0x91E1900Fu, 0x28163D58u,
    0x83D107A2u, 0x4EAC149Au, 0x9F71DA18u, 0x61D5C4FAu, 0xE3AB2A5Fu, 0xC7B0D63Fu, 0xB3CC752Au, 0x61EBCFB6u,
    0x26FFB52Au, 0xED789E3Fu, 0xAA3BC958u, 0x455A8788u, 0xC9C082A9u, 0x0A1BEF0Eu, 0xC29A5A7Eu, 0x150D4735u,
    0x943809E0u, 0x69215510u, 0xEF0B0DA9u, 0x3B4E9FB3u, 0xD8B5D04Cu, 0xC7A023A8u, 0xB0D50288u, 0x64821375u,
    0xC260E8CFu, 0x8496BD2Cu, 0xFF4F5435u, 0x0FB5560Cu, 0x7CD74A52u, 0x93589C80u, 0x88975C47u, 0x83BDA89Du,
    0x8BCC4296u, 0x01B82C21u, 0xFD821DBFu, 0x26520B47u, 0x04983E19u, 0xD3E1CA27u, 0x782C580Fu, 0x326FF573u,
    0xC157BCC7u, 0x4F5E6B84u, 0x44EBFBFBu, 0xDA26D9D8u, 0x6CD9D08Eu, 0x1719F1D8u, 0x715C0487u, 0x2C2D3C92u,
    0x53FAABA9u, 0xBC836146u, 0x510C92D6u, 0xE089F82Au, 0x4680171Fu, 0x369F00DEu, 0x70EC2331u, 0x0E253D55u,
    0xDAFB9717u, 0xE5DD922Du, 0x95915D21u, 0xA0202F96u, 0xA161CC47u, 0xEACFA6F1u, 0xED5E9189u, 0xDAB87684u,
    0xA4B76D4Au, 0xFA704897u, 0x631F10BAu, 0xD39DA8F9u, 0x5DB4C0E4u, 0x16FDE42Au, 0x2DFF7580u, 0xB56FEC7Eu,
    0xC3FFB370u, 0x8E6F36BCu, 0x6097D459u, 0x514D5D36u, 0xA5A737E2u, 0x3977B9B3u, 0xFD31A0CAu, 0x903368DBu,
    0xE8370D61u, 0x98109520u, 0xADE23CACu, 0x99F82E04u, 0x41DE7EA3u, 0x84A1C295u, 0x09191BE0u, 0x30930D02u,
    0x1C9FA44Au, 0xC406B6D7u, 0xEEDCA152u, 0x6149809Cu, 0xB0099EF4u, 0xC5F653A5u, 0x4C10790Du, 0x7303286Cu,
};

static const uint32_t kCrc32Table[256] = {
    0x00000000u, 0x77073096u, 0xEE0E612Cu, 0x990951BAu, 0x076DC419u, 0x706AF48Fu, 0xE963A535u, 
    0x9E6495A3u, 0x0EDB8832u, 0x79DCB8A4u, 0xE0D5E91Eu, 0x97D2D988u, 0x09B64C2Bu, 0x7EB17CBDu, 
    0xE7B82D07u, 0x90BF1D91u, 0x1DB71064u, 0x6AB020F2u, 0xF3B97148u, 0x84BE41DEu, 0x1ADAD47Du, 
    0x6DDDE4EBu, 0xF4D4B551u, 0x83D385C7u, 0x136C9856u, 0x646BA8C0u, 0xFD62F97Au, 0x8A65C9ECu, 
    0x14015C4Fu, 0x63066CD9u, 0xFA0F3D63u, 0x8D080DF5u, 0x3B6E20C8u, 0x4C69105Eu, 0xD56041E4u, 
    0xA2677172u, 0x3C03E4D1u, 0x4B04D447u, 0xD20D85FDu, 0xA50AB56Bu, 0x35B5A8FAu, 0x42B2986Cu, 
    0xDBBBC9D6u, 0xACBCF940u, 0x32D86CE3u, 0x45DF5C75u, 0xDCD60DCFu, 0xABD13D59u, 0x26D930ACu, 
    0x51DE003Au, 0xC8D75180u, 0xBFD06116u, 0x21B4F4B5u, 0x56B3C423u, 0xCFBA9599u, 0xB8BDA50Fu, 
    0x2802B89Eu, 0x5F058808u, 0xC60CD9B2u, 0xB10BE924u, 0x2F6F7C87u, 0x58684C11u, 0xC1611DABu, 
    0xB6662D3Du, 0x76DC4190u, 0x01DB7106u, 0x98D220BCu, 0xEFD5102Au, 0x71B18589u, 0x06B6B51Fu, 
    0x9FBFE4A5u, 0xE8B8D433u, 0x7807C9A2u, 0x0F00F934u, 0x9609A88Eu, 0xE10E9818u, 0x7F6A0DBBu, 
    0x086D3D2Du, 0x91646C97u, 0xE6635C01u, 0x6B6B51F4u, 0x1C6C6162u, 0x856530D8u, 0xF262004Eu, 
    0x6C0695EDu, 0x1B01A57Bu, 0x8208F4C1u, 0xF50FC457u, 0x65B0D9C6u, 0x12B7E950u, 0x8BBEB8EAu, 
    0xFCB9887Cu, 0x62DD1DDFu, 0x15DA2D49u, 0x8CD37CF3u, 0xFBD44C65u, 0x4DB26158u, 0x3AB551CEu, 
    0xA3BC0074u, 0xD4BB30E2u, 0x4ADFA541u, 0x3DD895D7u, 0xA4D1C46Du, 0xD3D6F4FBu, 0x4369E96Au, 
    0x346ED9FCu, 0xAD678846u, 0xDA60B8D0u, 0x44042D73u, 0x33031DE5u, 0xAA0A4C5Fu, 0xDD0D7CC9u, 
    0x5005713Cu, 0x270241AAu, 0xBE0B1010u, 0xC90C2086u, 0x5768B525u, 0x206F85B3u, 0xB966D409u, 
    0xCE61E49Fu, 0x5EDEF90Eu, 0x29D9C998u, 0xB0D09822u, 0xC7D7A8B4u, 0x59B33D17u, 0x2EB40D81u, 
    0xB7BD5C3Bu, 0xC0BA6CADu, 0xEDB88320u, 0x9ABFB3B6u, 0x03B6E20Cu, 0x74B1D29Au, 0xEAD54739u, 
    0x9DD277AFu, 0x04DB2615u, 0x73DC1683u, 0xE3630B12u, 0x94643B84u, 0x0D6D6A3Eu, 0x7A6A5AA8u, 
    0xE40ECF0Bu, 0x9309FF9Du, 0x0A00AE27u, 0x7D079EB1u, 0xF00F9344u, 0x8708A3D2u, 0x1E01F268u, 
    0x6906C2FEu, 0xF762575Du, 0x806567CBu, 0x196C3671u, 0x6E6B06E7u, 0xFED41B76u, 0x89D32BE0u, 
    0x10DA7A5Au, 0x67DD4ACCu, 0xF9B9DF6Fu, 0x8EBEEFF9u, 0x17B7BE43u, 0x60B08ED5u, 0xD6D6A3E8u, 
    0xA1D1937Eu, 0x38D8C2C4u, 0x4FDFF252u, 0xD1BB67F1u, 0xA6BC5767u, 0x3FB506DDu, 0x48B2364Bu, 
    0xD80D2BDAu, 0xAF0A1B4Cu, 0x36034AF6u, 0x41047A60u, 0xDF60EFC3u, 0xA867DF55u, 0x316E8EEFu, 
    0x4669BE79u, 0xCB61B38Cu, 0xBC66831Au, 0x256FD2A0u, 0x5268E236u, 0xCC0C7795u, 0xBB0B4703u, 
    0x220216B9u, 0x5505262Fu, 0xC5BA3BBEu, 0xB2BD0B28u, 0x2BB45A92u, 0x5CB36A04u, 0xC2D7FFA7u, 
    0xB5D0CF31u, 0x2CD99E8Bu, 0x5BDEAE1Du, 0x9B64C2B0u, 0xEC63F226u, 0x756AA39Cu, 0x026D930Au, 
    0x9C0906A9u, 0xEB0E363Fu, 0x72076785u, 0x05005713u, 0x95BF4A82u, 0xE2B87A14u, 0x7BB12BAEu, 
    0x0CB61B38u, 0x92D28E9Bu, 0xE5D5BE0Du, 0x7CDCEFB7u, 0x0BDBDF21u, 0x86D3D2D4u, 0xF1D4E242u, 
    0x68DDB3F8u, 0x1FDA836Eu, 0x81BE16CDu, 0xF6B9265Bu, 0x6FB077E1u, 0x18B74777u, 0x88085AE6u, 
    0xFF0F6A70u, 0x66063BCAu, 0x11010B5Cu, 0x8F659EFFu, 0xF862AE69u, 0x616BFFD3u, 0x166CCF45u, 
    0xA00AE278u, 0xD70DD2EEu, 0x4E048354u, 0x3903B3C2u, 0xA7672661u, 0xD06016F7u, 0x4969474Du, 
    0x3E6E77DBu, 0xAED16A4Au, 0xD9D65ADCu, 0x40DF0B66u, 0x37D83BF0u, 0xA9BCAE53u, 0xDEBB9EC5u, 
    0x47B2CF7Fu, 0x30B5FFE9u, 0xBDBDF21Cu, 0xCABAC28Au, 0x53B39330u, 0x24B4A3A6u, 0xBAD03605u, 
    0xCDD70693u, 0x54DE5729u, 0x23D967BFu, 0xB3667A2Eu, 0xC4614AB8u, 0x5D681B02u, 0x2A6F2B94u, 
    0xB40BBE37u, 0xC30C8EA1u, 0x5A05DF1Bu, 0x2D02EF8D
};

const uint32_t* details::GetMpqCryptTable()noexcept
{
    return kMpqCryptTable;
}

const uint32_t* details::GetCrc32Table()noexcept
{
    return kCrc32Table;
}

//////////////////////////////////////////////////////////////////////////////// Md5

// 基本的MD5操作
#define MD5_F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define MD5_G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))

// 单次变换操作
#define MD5_STEP(f, a, b, c, d, x, t, s) \
    do { \
        (a) += f((b), (c), (d)) + (x) + (t); \
        (a) = (((a) << (s)) | (((a) & 0xFFFFFFFFu) >> (32u - (s)))); \
        (a) += (b); \
    } while (false)

// 获取小端序
#define MD5_READ_LE(arr, i) \
    ((arr)[(i)] | (static_cast<uint32_t>((arr)[(i) + 1]) << 8u) | (static_cast<uint32_t>((arr)[(i) + 2]) << 16u) | \
    (static_cast<uint32_t>((arr)[(i) + 3] << 24u)))

Md5& Md5::Update(BytesView input)noexcept
{
    assert(m_iState == STATE_DEFAULT);
    assert(input.GetSize() <= numeric_limits<uint32_t>::max());

    auto lo = m_uLo;
    auto data = input.GetBuffer();
    auto size = static_cast<uint32_t>(input.GetSize());
    if ((m_uLo = (lo + size) & 0x1FFFFFFFu) < lo)
        ++m_uHi;
    m_uHi += (size >> 29u);

    auto used = lo & 0x3Fu;
    if (used)
    {
        auto free = 64u - used;

        if (size < free)
        {
            ::memcpy(&m_stBuffer[used], data, size);
            return *this;
        }

        ::memcpy(&m_stBuffer[used], data, free);
        data += free;
        size -= free;
        Transform(m_stBuffer.data(), m_stBuffer.size());
    }

    if (size >= 64)
    {
        data = Transform(data, size & ~0x3Fu);
        size &= 0x3F;
    }

    ::memcpy(m_stBuffer.data(), data, size);
    return *this;
}

const Md5::ResultType& Md5::Final()noexcept
{
    if (m_iState == STATE_FINISHED)
        return m_stResult;

    auto used = m_uLo & 0x3Fu;
    m_stBuffer[used++] = 0x80u;
    auto free = 64u - used;

    if (free < 8)
    {
        ::memset(&m_stBuffer[used], 0, free);
        Transform(m_stBuffer.data(), m_stBuffer.size());
        used = 0;
        free = 64;
    }

    ::memset(&m_stBuffer[used], 0, free - 8);

    m_uLo <<= 3u;
    m_stBuffer[56] = static_cast<uint8_t>(m_uLo & 0xFFu);
    m_stBuffer[57] = static_cast<uint8_t>((m_uLo >> 8u) & 0xFFu);
    m_stBuffer[58] = static_cast<uint8_t>((m_uLo >> 16u) & 0xFFu);
    m_stBuffer[59] = static_cast<uint8_t>((m_uLo >> 24u) & 0xFFu);
    m_stBuffer[60] = static_cast<uint8_t>(m_uHi & 0xFFu);
    m_stBuffer[61] = static_cast<uint8_t>((m_uHi >> 8u) & 0xFFu);
    m_stBuffer[62] = static_cast<uint8_t>((m_uHi >> 16u) & 0xFFu);
    m_stBuffer[63] = static_cast<uint8_t>((m_uHi >> 24u) & 0xFFu);

    Transform(m_stBuffer.data(), m_stBuffer.size());

    m_stResult[0]  = static_cast<uint8_t>(m_uA & 0xFFu);
    m_stResult[1]  = static_cast<uint8_t>((m_uA >> 8u) & 0xFFu);
    m_stResult[2]  = static_cast<uint8_t>((m_uA >> 16u) & 0xFFu);
    m_stResult[3]  = static_cast<uint8_t>((m_uA >> 24u) & 0xFFu);
    m_stResult[4]  = static_cast<uint8_t>(m_uB & 0xFFu);
    m_stResult[5]  = static_cast<uint8_t>((m_uB >> 8u) & 0xFFu);
    m_stResult[6]  = static_cast<uint8_t>((m_uB >> 16u) & 0xFFu);
    m_stResult[7]  = static_cast<uint8_t>((m_uB >> 24u) & 0xFFu);
    m_stResult[8]  = static_cast<uint8_t>(m_uC & 0xFFu);
    m_stResult[9]  = static_cast<uint8_t>((m_uC >> 8u) & 0xFFu);
    m_stResult[10] = static_cast<uint8_t>((m_uC >> 16u) & 0xFFu);
    m_stResult[11] = static_cast<uint8_t>((m_uC >> 24u) & 0xFFu);
    m_stResult[12] = static_cast<uint8_t>(m_uD & 0xFFu);
    m_stResult[13] = static_cast<uint8_t>((m_uD >> 8u) & 0xFFu);
    m_stResult[14] = static_cast<uint8_t>((m_uD >> 16u) & 0xFFu);
    m_stResult[15] = static_cast<uint8_t>((m_uD >> 24u) & 0xFFu);
    return m_stResult;
}

const uint8_t* Md5::Transform(const uint8_t* data, size_t length)noexcept
{
    assert(length % 64 == 0);

    auto a = m_uA;
    auto b = m_uB;
    auto c = m_uC;
    auto d = m_uD;

    do
    {
        auto savedA = a;
        auto savedB = b;
        auto savedC = c;
        auto savedD = d;

        uint32_t chunk[16] = {
            MD5_READ_LE(data, 0u),
            MD5_READ_LE(data, 4u),
            MD5_READ_LE(data, 8u),
            MD5_READ_LE(data, 12u),
            MD5_READ_LE(data, 16u),
            MD5_READ_LE(data, 20u),
            MD5_READ_LE(data, 24u),
            MD5_READ_LE(data, 28u),
            MD5_READ_LE(data, 32u),
            MD5_READ_LE(data, 36u),
            MD5_READ_LE(data, 40u),
            MD5_READ_LE(data, 44u),
            MD5_READ_LE(data, 48u),
            MD5_READ_LE(data, 52u),
            MD5_READ_LE(data, 56u),
            MD5_READ_LE(data, 60u),
        };

        // Round 1
        MD5_STEP(MD5_F, a, b, c, d, chunk[0],  0xD76AA478u, 7u);
        MD5_STEP(MD5_F, d, a, b, c, chunk[1],  0xE8C7B756u, 12u);
        MD5_STEP(MD5_F, c, d, a, b, chunk[2],  0x242070DBu, 17u);
        MD5_STEP(MD5_F, b, c, d, a, chunk[3],  0xC1BDCEEEu, 22u);
        MD5_STEP(MD5_F, a, b, c, d, chunk[4],  0xF57C0FAFu, 7u);
        MD5_STEP(MD5_F, d, a, b, c, chunk[5],  0x4787C62Au, 12u);
        MD5_STEP(MD5_F, c, d, a, b, chunk[6],  0xA8304613u, 17u);
        MD5_STEP(MD5_F, b, c, d, a, chunk[7],  0xFD469501u, 22u);
        MD5_STEP(MD5_F, a, b, c, d, chunk[8],  0x698098D8u, 7u);
        MD5_STEP(MD5_F, d, a, b, c, chunk[9],  0x8B44F7AFu, 12u);
        MD5_STEP(MD5_F, c, d, a, b, chunk[10], 0xFFFF5BB1u, 17u);
        MD5_STEP(MD5_F, b, c, d, a, chunk[11], 0x895CD7BEu, 22u);
        MD5_STEP(MD5_F, a, b, c, d, chunk[12], 0x6B901122u, 7u);
        MD5_STEP(MD5_F, d, a, b, c, chunk[13], 0xFD987193u, 12u);
        MD5_STEP(MD5_F, c, d, a, b, chunk[14], 0xA679438Eu, 17u);
        MD5_STEP(MD5_F, b, c, d, a, chunk[15], 0x49B40821u, 22u);

        // Round 2
        MD5_STEP(MD5_G, a, b, c, d, chunk[1],  0xF61E2562u, 5u);
        MD5_STEP(MD5_G, d, a, b, c, chunk[6],  0xC040B340u, 9u);
        MD5_STEP(MD5_G, c, d, a, b, chunk[11], 0x265E5A51u, 14u);
        MD5_STEP(MD5_G, b, c, d, a, chunk[0],  0xE9B6C7AAu, 20u);
        MD5_STEP(MD5_G, a, b, c, d, chunk[5],  0xD62F105Du, 5u);
        MD5_STEP(MD5_G, d, a, b, c, chunk[10], 0x02441453u, 9u);
        MD5_STEP(MD5_G, c, d, a, b, chunk[15], 0xD8A1E681u, 14u);
        MD5_STEP(MD5_G, b, c, d, a, chunk[4],  0xE7D3FBC8u, 20u);
        MD5_STEP(MD5_G, a, b, c, d, chunk[9],  0x21E1CDE6u, 5u);
        MD5_STEP(MD5_G, d, a, b, c, chunk[14], 0xC33707D6u, 9u);
        MD5_STEP(MD5_G, c, d, a, b, chunk[3],  0xF4D50D87u, 14u);
        MD5_STEP(MD5_G, b, c, d, a, chunk[8],  0x455A14EDu, 20u);
        MD5_STEP(MD5_G, a, b, c, d, chunk[13], 0xA9E3E905u, 5u);
        MD5_STEP(MD5_G, d, a, b, c, chunk[2],  0xFCEFA3F8u, 9u);
        MD5_STEP(MD5_G, c, d, a, b, chunk[7],  0x676F02D9u, 14u);
        MD5_STEP(MD5_G, b, c, d, a, chunk[12], 0x8D2A4C8Au, 20u);

        // Round 3
        MD5_STEP(MD5_H, a, b, c, d, chunk[5],  0xFFFA3942u, 4u);
        MD5_STEP(MD5_H, d, a, b, c, chunk[8],  0x8771F681u, 11u);
        MD5_STEP(MD5_H, c, d, a, b, chunk[11], 0x6D9D6122u, 16u);
        MD5_STEP(MD5_H, b, c, d, a, chunk[14], 0xFDE5380Cu, 23u);
        MD5_STEP(MD5_H, a, b, c, d, chunk[1],  0xA4BEEA44u, 4u);
        MD5_STEP(MD5_H, d, a, b, c, chunk[4],  0x4BDECFA9u, 11u);
        MD5_STEP(MD5_H, c, d, a, b, chunk[7],  0xF6BB4B60u, 16u);
        MD5_STEP(MD5_H, b, c, d, a, chunk[10], 0xBEBFBC70u, 23u);
        MD5_STEP(MD5_H, a, b, c, d, chunk[13], 0x289B7EC6u, 4u);
        MD5_STEP(MD5_H, d, a, b, c, chunk[0],  0xEAA127FAu, 11u);
        MD5_STEP(MD5_H, c, d, a, b, chunk[3],  0xD4EF3085u, 16u);
        MD5_STEP(MD5_H, b, c, d, a, chunk[6],  0x04881D05u, 23u);
        MD5_STEP(MD5_H, a, b, c, d, chunk[9],  0xD9D4D039u, 4u);
        MD5_STEP(MD5_H, d, a, b, c, chunk[12], 0xE6DB99E5u, 11u);
        MD5_STEP(MD5_H, c, d, a, b, chunk[15], 0x1FA27CF8u, 16u);
        MD5_STEP(MD5_H, b, c, d, a, chunk[2],  0xC4AC5665u, 23u);

        // Round 4
        MD5_STEP(MD5_I, a, b, c, d, chunk[0],  0xF4292244u, 6u);
        MD5_STEP(MD5_I, d, a, b, c, chunk[7],  0x432AFF97u, 10u);
        MD5_STEP(MD5_I, c, d, a, b, chunk[14], 0xAB9423A7u, 15u);
        MD5_STEP(MD5_I, b, c, d, a, chunk[5],  0xFC93A039u, 21u);
        MD5_STEP(MD5_I, a, b, c, d, chunk[12], 0x655B59C3u, 6u);
        MD5_STEP(MD5_I, d, a, b, c, chunk[3],  0x8F0CCC92u, 10u);
        MD5_STEP(MD5_I, c, d, a, b, chunk[10], 0xFFEFF47Du, 15u);
        MD5_STEP(MD5_I, b, c, d, a, chunk[1],  0x85845DD1u, 21u);
        MD5_STEP(MD5_I, a, b, c, d, chunk[8],  0x6FA87E4Fu, 6u);
        MD5_STEP(MD5_I, d, a, b, c, chunk[15], 0xFE2CE6E0u, 10u);
        MD5_STEP(MD5_I, c, d, a, b, chunk[6],  0xA3014314u, 15u);
        MD5_STEP(MD5_I, b, c, d, a, chunk[13], 0x4E0811A1u, 21u);
        MD5_STEP(MD5_I, a, b, c, d, chunk[4],  0xF7537E82u, 6u);
        MD5_STEP(MD5_I, d, a, b, c, chunk[11], 0xBD3AF235u, 10u);
        MD5_STEP(MD5_I, c, d, a, b, chunk[2],  0x2AD7D2BBu, 15u);
        MD5_STEP(MD5_I, b, c, d, a, chunk[9],  0xEB86D391u, 21u);

        a += savedA;
        b += savedB;
        c += savedC;
        d += savedD;

        data += 64;
        length -= 64;
    } while(length > 0);

    m_uA = a;
    m_uB = b;
    m_uC = c;
    m_uD = d;
    return data;
}

//////////////////////////////////////////////////////////////////////////////// Sha1

#define SHA1_LOAD32H(x, y) x = (static_cast<uint32_t>((y)[0] & 255u) << 24u) | \
    (static_cast<uint32_t>((y)[1] & 255u) << 16u) | (static_cast<uint32_t>((y)[2] & 255u) << 8u) | \
    (static_cast<uint32_t>((y)[3] & 255u));

#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32u - (bits))))

#define SHA1_BLK0(i) block->l[i]

#define SHA1_BLK(i) \
    (block->l[i & 15u] = SHA1_ROL(block->l[(i + 13u) & 15u] ^ block->l[(i + 8u) & 15u] ^ block->l[(i + 2u) & 15u] ^ \
    block->l[i & 15u], 1u))

#define SHA1_R0(v, w, x, y, z, i) z += ((w & (x ^ y)) ^ y) + SHA1_BLK0(i)+ 0x5A827999u + SHA1_ROL(v, 5u); \
    w = SHA1_ROL(w, 30u);
#define SHA1_R1(v, w, x, y, z, i) z += ((w & (x ^ y)) ^ y) + SHA1_BLK(i) + 0x5A827999u + SHA1_ROL(v, 5u); \
    w = SHA1_ROL(w, 30u);
#define SHA1_R2(v, w, x, y, z, i) z += (w ^ x ^ y) + SHA1_BLK(i) + 0x6ED9EBA1u + SHA1_ROL(v, 5u); w = SHA1_ROL(w, 30u);
#define SHA1_R3(v, w, x, y, z, i) z += (((w | x) & y) | (w & x)) + SHA1_BLK(i) + 0x8F1BBCDCu + SHA1_ROL(v, 5u); \
    w = SHA1_ROL(w, 30u);
#define SHA1_R4(v, w, x, y, z, i) z += (w ^ x ^ y) + SHA1_BLK(i) + 0xCA62C1D6u + SHA1_ROL(v, 5u); w = SHA1_ROL(w, 30u);

Sha1& Sha1::Update(BytesView input)noexcept
{
    assert(m_iState == STATE_DEFAULT);

    uint32_t i = 0;
    uint32_t j = (m_uCount[0] >> 3u) & 63u;
    if ((m_uCount[0] += static_cast<uint32_t>(input.GetSize() << 3u)) < (input.GetSize() << 3u))
        ++m_uCount[1];

    m_uCount[1] += static_cast<uint32_t>(input.GetSize() >> 29u);
    if ((j + input.GetSize()) > 63u)
    {
        i = 64u - j;
        ::memcpy(&m_stBuffer[j], input.GetBuffer(), i);
        Transform(m_stBuffer.data());
        for (; i + 63u < input.GetSize(); i += 64u)
            Transform(&input[i]);
        j = 0;
    }

    if (input.GetSize() != i)
        ::memcpy(&m_stBuffer[j], &input[i], input.GetSize() - i);
    return *this;
}

const Sha1::ResultType& Sha1::Final()noexcept
{
    if (m_iState == STATE_FINISHED)
        return m_stResult;

    uint8_t final[8];
    for (uint32_t i = 0; i < 8; ++i)
        final[i] = static_cast<uint8_t>((m_uCount[(i >= 4 ? 0 : 1)] >> ((3u - (i & 3u)) * 8u)) & 255u);
    Update(BytesView(reinterpret_cast<const uint8_t*>("\x80"), 1));
    while ((m_uCount[0] & 504u) != 448u)
        Update(BytesView(reinterpret_cast<const uint8_t*>("\0"), 1));
    Update(BytesView(final, 8));
    for (uint32_t i = 0; i < kHashSize; ++i)
        m_stResult[i] = static_cast<uint8_t>((m_uState[i >> 2u] >> ((3u - (i & 3u)) * 8u)) & 255u);
    m_iState = STATE_FINISHED;
    return m_stResult;
}

void Sha1::Transform(const uint8_t buffer[64])noexcept
{
    union BlockType
    {
        uint8_t c[64];
        uint32_t l[16];
    };

    uint32_t a = m_uState[0];
    uint32_t b = m_uState[1];
    uint32_t c = m_uState[2];
    uint32_t d = m_uState[3];
    uint32_t e = m_uState[4];
    uint8_t workspace[64];
    auto* block = reinterpret_cast<BlockType*>(workspace);

    for (uint32_t i = 0; i < 16; ++i)
        SHA1_LOAD32H(block->l[i], buffer + (i * 4));

    SHA1_R0(a,b,c,d,e, 0u); SHA1_R0(e,a,b,c,d, 1u); SHA1_R0(d,e,a,b,c, 2u); SHA1_R0(c,d,e,a,b, 3u);
    SHA1_R0(b,c,d,e,a, 4u); SHA1_R0(a,b,c,d,e, 5u); SHA1_R0(e,a,b,c,d, 6u); SHA1_R0(d,e,a,b,c, 7u);
    SHA1_R0(c,d,e,a,b, 8u); SHA1_R0(b,c,d,e,a, 9u); SHA1_R0(a,b,c,d,e,10u); SHA1_R0(e,a,b,c,d,11u);
    SHA1_R0(d,e,a,b,c,12u); SHA1_R0(c,d,e,a,b,13u); SHA1_R0(b,c,d,e,a,14u); SHA1_R0(a,b,c,d,e,15u);
    SHA1_R1(e,a,b,c,d,16u); SHA1_R1(d,e,a,b,c,17u); SHA1_R1(c,d,e,a,b,18u); SHA1_R1(b,c,d,e,a,19u);
    SHA1_R2(a,b,c,d,e,20u); SHA1_R2(e,a,b,c,d,21u); SHA1_R2(d,e,a,b,c,22u); SHA1_R2(c,d,e,a,b,23u);
    SHA1_R2(b,c,d,e,a,24u); SHA1_R2(a,b,c,d,e,25u); SHA1_R2(e,a,b,c,d,26u); SHA1_R2(d,e,a,b,c,27u);
    SHA1_R2(c,d,e,a,b,28u); SHA1_R2(b,c,d,e,a,29u); SHA1_R2(a,b,c,d,e,30u); SHA1_R2(e,a,b,c,d,31u);
    SHA1_R2(d,e,a,b,c,32u); SHA1_R2(c,d,e,a,b,33u); SHA1_R2(b,c,d,e,a,34u); SHA1_R2(a,b,c,d,e,35u);
    SHA1_R2(e,a,b,c,d,36u); SHA1_R2(d,e,a,b,c,37u); SHA1_R2(c,d,e,a,b,38u); SHA1_R2(b,c,d,e,a,39u);
    SHA1_R3(a,b,c,d,e,40u); SHA1_R3(e,a,b,c,d,41u); SHA1_R3(d,e,a,b,c,42u); SHA1_R3(c,d,e,a,b,43u);
    SHA1_R3(b,c,d,e,a,44u); SHA1_R3(a,b,c,d,e,45u); SHA1_R3(e,a,b,c,d,46u); SHA1_R3(d,e,a,b,c,47u);
    SHA1_R3(c,d,e,a,b,48u); SHA1_R3(b,c,d,e,a,49u); SHA1_R3(a,b,c,d,e,50u); SHA1_R3(e,a,b,c,d,51u);
    SHA1_R3(d,e,a,b,c,52u); SHA1_R3(c,d,e,a,b,53u); SHA1_R3(b,c,d,e,a,54u); SHA1_R3(a,b,c,d,e,55u);
    SHA1_R3(e,a,b,c,d,56u); SHA1_R3(d,e,a,b,c,57u); SHA1_R3(c,d,e,a,b,58u); SHA1_R3(b,c,d,e,a,59u);
    SHA1_R4(a,b,c,d,e,60u); SHA1_R4(e,a,b,c,d,61u); SHA1_R4(d,e,a,b,c,62u); SHA1_R4(c,d,e,a,b,63u);
    SHA1_R4(b,c,d,e,a,64u); SHA1_R4(a,b,c,d,e,65u); SHA1_R4(e,a,b,c,d,66u); SHA1_R4(d,e,a,b,c,67u);
    SHA1_R4(c,d,e,a,b,68u); SHA1_R4(b,c,d,e,a,69u); SHA1_R4(a,b,c,d,e,70u); SHA1_R4(e,a,b,c,d,71u);
    SHA1_R4(d,e,a,b,c,72u); SHA1_R4(c,d,e,a,b,73u); SHA1_R4(b,c,d,e,a,74u); SHA1_R4(a,b,c,d,e,75u);
    SHA1_R4(e,a,b,c,d,76u); SHA1_R4(d,e,a,b,c,77u); SHA1_R4(c,d,e,a,b,78u); SHA1_R4(b,c,d,e,a,79u);

    m_uState[0] += a;
    m_uState[1] += b;
    m_uState[2] += c;
    m_uState[3] += d;
    m_uState[4] += e;
}

//////////////////////////////////////////////////////////////////////////////// Sha256

static const uint32_t kSha256Table[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu,
    0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u,
    0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u,
    0xc19bf174u, 0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau, 0x983e5152u,
    0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu,
    0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u,
    0xd6990624u, 0xf40e3585u, 0x106aa070u, 0x19a4c116u, 0x1e376c08u,
    0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu,
    0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define SHA256_STORE32H(x, y) \
    do { \
        (y)[0] = static_cast<uint8_t>(((x) >> 24u) & 255u); (y)[1] = static_cast<uint8_t>(((x) >> 16u) & 255u); \
        (y)[2] = static_cast<uint8_t>(((x) >> 8u) & 255u); (y)[3] = static_cast<uint8_t>((x) & 255u); \
    } while (false)

#define SHA256_LOAD32H(x, y) \
    do { \
        x = (static_cast<uint32_t>((y)[0] & 255u) << 24u) | (static_cast<uint32_t>((y)[1] & 255u) << 16u) | \
            (static_cast<uint32_t>((y)[2] & 255u) << 8u) | (static_cast<uint32_t>((y)[3] & 255u)); \
    } while (false)

#define SHA256_STORE64H(x, y) \
    do { \
        (y)[0] = static_cast<uint8_t>(((x) >> 56u) & 255u); (y)[1] = static_cast<uint8_t>(((x) >> 48u) & 255u); \
        (y)[2] = static_cast<uint8_t>(((x) >> 40u) & 255u); (y)[3] = static_cast<uint8_t>(((x) >> 32u) & 255u); \
        (y)[4] = static_cast<uint8_t>(((x) >> 24u) & 255u); (y)[5] = static_cast<uint8_t>(((x) >> 16u) & 255u); \
        (y)[6] = static_cast<uint8_t>(((x) >> 8u) & 255u); (y)[7] = static_cast<uint8_t>((x) & 255u); \
    } while (false)

#define SHA256_ROR(value, bits) (((value) >> (bits)) | ((value) << (32u - (bits))))
#define SHA256_CH(x, y, z) (z ^ (x & (y ^ z)))
#define SHA256_MAJ(x, y, z) (((x | y) & z) | (x & y))
#define SHA256_S(x, n) SHA256_ROR((x), (n))
#define SHA256_R(x, n) (((x) & 0xFFFFFFFFu) >> (n))
#define SHA256_SIGMA0(x) (SHA256_S(x, 2) ^ SHA256_S(x, 13) ^ SHA256_S(x, 22))
#define SHA256_SIGMA1(x) (SHA256_S(x, 6) ^ SHA256_S(x, 11) ^ SHA256_S(x, 25))
#define SHA256_GAMMA0(x) (SHA256_S(x, 7) ^ SHA256_S(x, 18) ^ SHA256_R(x, 3))
#define SHA256_GAMMA1(x) (SHA256_S(x, 17) ^ SHA256_S(x, 19) ^ SHA256_R(x, 10))

#define SHA256_ROUND(a, b, c, d, e, f, g, h, i) \
    do { \
        t0 = h + SHA256_SIGMA1(e) + SHA256_CH(e, f, g) + kSha256Table[i] + w[i]; \
        t1 = SHA256_SIGMA0(a) + SHA256_MAJ(a, b, c); \
        d += t0; \
        h  = t0 + t1; \
    } while (false)

Sha256& Sha256::Update(BytesView input)noexcept
{
    assert(m_iState == STATE_DEFAULT);
    assert(m_uCurrent < m_stBuffer.size());

    auto size = input.GetSize();
    auto buffer = input.GetBuffer();
    while (size > 0)
    {
        if (m_uCurrent == 0 && size >= kBlockSize)
        {
            Transform(buffer);
            m_uLength += kBlockSize * 8;
            buffer += kBlockSize;
            size -= kBlockSize;
        }
        else
        {
            auto n = std::min<uint32_t>(static_cast<uint32_t>(size), kBlockSize - m_uCurrent);
            ::memcpy(m_stBuffer.data() + m_uCurrent, buffer, n);
            m_uCurrent += n;
            buffer += n;
            size -= n;
            if (m_uCurrent == kBlockSize)
            {
                Transform(m_stBuffer.data());
                m_uLength += 8 * kBlockSize;
                m_uCurrent = 0;
            }
        }
    }
    return *this;
}

const Sha256::ResultType& Sha256::Final()noexcept
{
    if (m_iState == STATE_FINISHED)
        return m_stResult;

    assert(m_uCurrent < m_stBuffer.size());
    m_uLength += m_uCurrent * 8;
    m_stBuffer[m_uCurrent++] = 0x80;

    if (m_uCurrent > 56)
    {
        while (m_uCurrent < 64)
            m_stBuffer[m_uCurrent++] = 0;
        Transform(m_stBuffer.data());
        m_uCurrent = 0;
    }

    while (m_uCurrent < 56)
        m_stBuffer[m_uCurrent++] = 0;

    SHA256_STORE64H(m_uLength, m_stBuffer.data() + 56);
    Transform(m_stBuffer.data());

    for (uint32_t i = 0; i < 8; ++i)
        SHA256_STORE32H(m_uState[i], &m_stResult[4 * i]);

    m_iState = STATE_FINISHED;
    return m_stResult;
}

void Sha256::Transform(const uint8_t* buffer)noexcept
{
    uint32_t s[8];
    uint32_t w[64];
    uint32_t t0;
    uint32_t t1;
    uint32_t t;

    for (uint32_t i = 0; i < 8; ++i)
        s[i] = m_uState[i];

    for (uint32_t i = 0; i < 16; ++i)
        SHA256_LOAD32H(w[i], buffer + (4 * i));

    for (uint32_t i = 16; i < 64; ++i)
        w[i] = SHA256_GAMMA1(w[i - 2]) + w[i - 7] + SHA256_GAMMA0(w[i - 15]) + w[i - 16];

    for (uint32_t i = 0; i < 64; ++i)
    {
        SHA256_ROUND(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], i);
        t = s[7];
        s[7] = s[6];
        s[6] = s[5];
        s[5] = s[4];
        s[4] = s[3];
        s[3] = s[2];
        s[2] = s[1];
        s[1] = s[0];
        s[0] = t;
    }

    for (uint32_t i = 0; i < 8; ++i)
        m_uState[i] = m_uState[i] + s[i];
}
