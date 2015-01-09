#include <stdlib.h>
#include <string.h>
#include "DBG.h"
#include "MEM.h"
#include "share.h"

/*
 * found_path size is FILENAME_MAX.
 */
static char *current_path;

char *
Ivory_get_current_path(void)
{
    return current_path;
}

void
Ivory_set_current_path(char *path)
{
	current_path = path;
}

SearchFileStatus
ISandBox_search_file(char *search_path, char *search_file,
                char *found_path, FILE **fp)
{
    int sp_idx;
    int dp_idx;
    char dir_path[FILENAME_MAX];
    char *home;
    int search_file_len;
    FILE *fp_tmp;

    search_file_len = strlen(search_file);

    for (sp_idx = 0, dp_idx = 0; ; sp_idx++) {
        if (search_path[sp_idx] == FILE_PATH_SEPARATOR
            || search_path[sp_idx] == '\0') {

            if (dp_idx + 1 + search_file_len >= FILENAME_MAX-1) {
                return SEARCH_FILE_PATH_TOO_LONG;
            }

            if (dp_idx > 0 && dir_path[dp_idx-1] != FILE_SEPARATOR) {
                dir_path[dp_idx] = FILE_SEPARATOR;
                dp_idx++;
            }
            strcpy(&dir_path[dp_idx], search_file);
            fp_tmp = fopen(dir_path, "r");
            if (fp_tmp != NULL) {
                *fp = fp_tmp;
                strcpy(found_path, dir_path);
                return SEARCH_FILE_SUCCESS;
            }
            dp_idx = 0;
            if (search_path[sp_idx] == '\0')
                return SEARCH_FILE_NOT_FOUND;
        } else {
            if (dp_idx == 0 && search_path[sp_idx] == '~'
                && (home = getenv("HOME")) != NULL) {
                strcpy(&dir_path[dp_idx], home);
                dp_idx += strlen(home);
            } else {
                if (dp_idx >= FILENAME_MAX-1) {
                    return SEARCH_FILE_PATH_TOO_LONG;
                }
                dir_path[dp_idx] = search_path[sp_idx];
                dp_idx++;
            }
        }
    }

    DBG_assert(0, ("bad flow."));
}

ISandBox_Boolean
ISandBox_compare_string(char *str1, char *str2)
{
    if (str1 == NULL && str2 == NULL)
        return ISandBox_TRUE;

    if (str1 == NULL || str2 == NULL)
        return ISandBox_FALSE;

    return !strcmp(str1, str2);
}

ISandBox_Boolean
ISandBox_compare_package_name(char *p1, char *p2)
{
    return ISandBox_compare_string(p1, p2);
}

char *
ISandBox_create_method_function_name(char *class_name, char *method_name)
{
    int class_name_len;
    int method_name_len;
    char *ret;

    class_name_len = strlen(class_name);
    method_name_len = strlen(method_name);
    ret = MEM_malloc(class_name_len + method_name_len + 2);
    sprintf(ret, "%s#%s", class_name, method_name);

    return ret;
}

void
ISandBox_strncpy(char *dest, char *src, int buf_size)
{
    int i;

    for (i = 0; i < (buf_size - 1) && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

#if 0
int main(int argc, char **argv)
{
    char *file_name = argv[1];
    char *search_path = getenv("PATH");
    FILE *fp;
    char found_path[FILENAME_MAX];
    SearchFileStatus status;

    status = ISandBox_search_file(search_path, file_name, found_path, &fp);

    fprintf(stderr, "status..%d\n", status);
    fprintf(stderr, "found_path..%s\n", found_path);
}
#endif

char *
ISandBox_get_folder_by_path(char *path)
{
	int length;
	int i;
	char *ret;

	length = strlen(path);
	for (i = length - 1;
		path[i] != '\\'
		&& path[i] != '/'
		&& i >= 0; i--);
	if (i != 0) {
		ret = (char *)malloc(sizeof(char) * (i + 1));
		strncpy(ret, path, i);
		ret[i] = NULL;
	} else {
		ret = ".";
	}
	return ret;
}

char *
ISandBox_get_absolute_path(char *relative_path)
{
	char *ret;
	char *root = Ivory_get_current_path();
	int length = strlen(root) + strlen(relative_path) + 1;

	ret = (char *)malloc(sizeof(char) * length);
	strcat(ret, root);
	strcat(ret, "/");
	strcat(ret, relative_path);
	ret[length-1] = NULL;

	return ret;
}
