#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_


#ifdef __cplusplus
extern "C" {
#endif	/* _cplusplus */


enum ParserState {
	PS_AssignmentSymbol,
	PS_CommentSymbol,
	PS_Seperators,
	PS_MaxLineLen
};

#define CFGOPT_INT	1
#define CFGOPT_FLT	2
	
struct ConfigOption {
	char *option, *str_value;
	int int_value;
	float flt_value;
	unsigned short flags;
};

void SetParserState(enum ParserState state, int value);
int LoadConfigFile(const char *fname);
const struct ConfigOption *GetNextOption();
void DestroyConfigParser();


#ifdef __cplusplus
}
#endif	/* _cplusplus */

#endif	/* _CONFIG_PARSER_H_ */
