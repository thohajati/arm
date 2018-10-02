#ifndef __SYSTEM_STATUSWORD_H__
#define __SYSTEM_STATUSWORD_H__

// 9.4.1	Responses to commands which are correctly executed

#define SW_OK				(unsigned short) 0x9000 // '90''00'	    normal ending of the command
#define SW_ERR_P3			(unsigned short) 0x6700 // '67''XX'	 	incorrect parameter P3 (see note)
#define SW_ERR_P1P2			(unsigned short) 0x6B00 // '6B''XX'#	incorrect parameter P1 or P2 (see ##)
#define SW_ERR_INS			(unsigned short) 0x6D00 // '6D''XX'#	unknown instruction code given in the command
#define SW_ERR_CLA			(unsigned short) 0x6E00 // '6E''XX'#	wrong instruction class given in the command
#define SW_ERR_TECH			(unsigned short) 0x6F00 // '6F''XX'#	technical problem with no diagnostic given
#define SW_NOT_FOUND        (unsigned short) 0x6A82

#endif /* __SYSTEM_STATUSWORD_H__ */
