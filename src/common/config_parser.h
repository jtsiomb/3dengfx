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

void set_parser_state(enum ParserState state, long value);
int load_config_file(const char *fname);
const struct ConfigOption *get_next_option();
void destroy_config_parser();


#ifdef __cplusplus
}
#endif	/* _cplusplus */

#endif	/* _CONFIG_PARSER_H_ */
