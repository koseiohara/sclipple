
char* trim(char* s);
int cat(char** out, char* a, char* b, char* delim);
int is_white_space(const char* line);
int line_to_dict(char* line, char** key, char** value);
void delete_bracket(char** s, int n, const char* lbracket, const char* rbracket);

