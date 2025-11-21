
/////   INCLUDES   /////

#include "advanced_help.h"




/////   FUNCTION DEFINITIONS   /////

char* getLineNode(_In_ char* new_line, _In_opt_ char* current_node);
WCHAR* getLineNodeW(_In_ WCHAR* new_line, _In_opt_ WCHAR* current_node);
size_t getNodeLevel(_In_ const char* current_node);
size_t getNodeLevelW(_In_ const WCHAR* current_node);




/////   FUNCTION IMPLEMENTATIONS   /////

// Finds all the nodes which contain the keyword and prints all parent sections and subsections like a tree
// The returned pointer must be freed by function caller
char* getAdvancedHelpForKeyword(_In_ const char* keyword, _In_ void* help_ptr) {
	char* help_to_show = NULL;
	char* full_help_text = NULL;
	char* current_nodes[MAX_NODE_LEVEL] = { 0 };
	bool current_nodes_already_included[MAX_NODE_LEVEL] = { 0 };

	if (NULL == help_ptr) {
		goto HELP_UNINITIALIZED_ERROR_LABEL;
	}

	full_help_text = (char*)malloc(strlen((char*)help_ptr) + 1);
	if (NULL == full_help_text) {
		goto HELP_NOMEM_ERROR_LABEL;
	}
	strcpy_s(full_help_text, strlen((char*)help_ptr) + 1, (char*)help_ptr);

	// Init arrays
	for (size_t i = 0; i < MAX_NODE_LEVEL; i++) {
		current_nodes[i] = NULL;
		current_nodes_already_included[i] = false;
	}

	if (0 == strcmp("", keyword)) {
		return full_help_text;
	}

	// Nodes are the same as lines, but without the NODE_START_CHAR if needed. No memory is allocated for lines nor nodes
	char* line = NULL;
	char* current_node = NULL;
	char* new_node = NULL;
	size_t current_node_level = 0;
	size_t forced_include_min_level = MAX_NODE_LEVEL;

	// Process all the help iteratively line by line
	// Note:
	//    When there are no more tokens, line becomes NULL, new_node becomes NULL too, and it will be different from current_node.
	//    Then the last node is processed.
	//    Finally, current_node is assigned to new_node (=NULL), allowing the loop to exit
	int iteration = 0;
	char* strtok_ctx = NULL;
	do {
		iteration++;
		//printf("\n\niteration %d\n", iteration);

		if (NULL == line) {
			line = strtok_s(full_help_text, "\n", &strtok_ctx);
		} else {
			line = strtok_s(NULL, "\n", &strtok_ctx);
		}
		new_node = getLineNode(line, current_node);
		//printf("  line:\n%s\n", line);
		//printf("  current_node:\n%s\n", current_node);
		//printf("  new_node:\n%s\n", new_node);

		//If current_node is NULL, assign it to the first node and continue, because it is the first iteration
		if (NULL == current_node) {
			current_node = new_node;
			continue;
		}
		// If the node is the same, continue reading because more tokens may fuse into the same node.
		if (new_node == current_node || NULL == current_node) {
			continue;
		}
		//printf("  Proceed to processing...\n");

		// Process current node (already complete with all its lines)
		current_node_level = getNodeLevel(current_node);
		//printf("  current_node_level=%llu\n", current_node_level);

		// Check that nodes do not skip levels (eg, a level 1 node followed by level 3 node without a level 2 node in between)
		for (size_t i = 0; i < current_node_level; i++) {
			if (NULL == current_nodes[i]) {
				//printf("current_nodes[%llu]=NULL\n", i);
				goto HELP_FORMAT_ERROR_LABEL;
			}
		}

		// Assign current node
		current_nodes[current_node_level] = current_node;
		current_nodes_already_included[current_node_level] = false;

		// Clear subnodes
		for (size_t i = current_node_level + 1; i < MAX_NODE_LEVEL; i++) {
			if (NULL == current_nodes[i]) {
				break;
			}
			current_nodes[i] = NULL;
			current_nodes_already_included[i] = false;
		}

		// Check if this node is forced to be added (due to parent node included the keyword)
		if (forced_include_min_level < current_node_level) {
			if ((0 != strAppendRealloc(&help_to_show, current_node)) || (0 != strAppendRealloc(&help_to_show, "\n"))) {
				goto HELP_NOMEM_ERROR_LABEL;
			}
		} else {
			// Stop forcing to include
			forced_include_min_level = MAX_NODE_LEVEL;

			// Search the keyword in the node
			if (NULL != strstr(current_node, keyword)) {	// Keyword found!
				// Include parent nodes if not already included
				for (size_t i = 0; i < current_node_level + 1; i++) {
					if (!current_nodes_already_included[i]) {
						if ((0 != strAppendRealloc(&help_to_show, current_nodes[i])) || (0 != strAppendRealloc(&help_to_show, "\n"))) {
							goto HELP_NOMEM_ERROR_LABEL;
						}
						current_nodes_already_included[i] = true;
					}
				}

				// Include current node (already done in the loop above?)
				//if ((0 != strAppendRealloc(&help_to_show, current_node)) || (0 != strAppendRealloc(&help_to_show, "\n"))) {
				//	goto HELP_NOMEM_ERROR_LABEL;
				//}
				//current_nodes_already_included[current_node_level] = true;

				// Force include everything below this level
				forced_include_min_level = current_node_level;
			}
		}

		current_node = new_node;
	} while (NULL != current_node);

	// No errors
	if (NULL == help_to_show) {
		goto HELP_KEYWORD_NOT_FOUND_LABEL;
	}
	return help_to_show;

HELP_UNINITIALIZED_ERROR_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	help_to_show = (char*)malloc(strlen(ADVANCED_HELP_UNINITIALIZED_ERROR) + 1);
	if (NULL == help_to_show) {
		goto HELP_NOMEM_ERROR_LABEL;
		//return NULL;	// Not even possible to output the error
	}
	strcpy_s(help_to_show, strlen(ADVANCED_HELP_UNINITIALIZED_ERROR) + 1, ADVANCED_HELP_UNINITIALIZED_ERROR);
	return help_to_show;


HELP_KEYWORD_NOT_FOUND_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	help_to_show = (char*)malloc(strlen(ADVANCED_HELP_KEYWORD_NOT_FOUND_INFO) + 1);
	if (NULL == help_to_show) {
		goto HELP_NOMEM_ERROR_LABEL;
		//return NULL;	// Not even possible to output the error
	}
	strcpy_s(help_to_show, strlen(ADVANCED_HELP_KEYWORD_NOT_FOUND_INFO) + 1, ADVANCED_HELP_KEYWORD_NOT_FOUND_INFO);
	return help_to_show;


HELP_FORMAT_ERROR_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	help_to_show = (char*)malloc(strlen(ADVANCED_HELP_FORMAT_ERROR) + 1);
	if (NULL == help_to_show) {
		goto HELP_NOMEM_ERROR_LABEL;
		//return NULL;	// Not even possible to output the error
	}
	strcpy_s(help_to_show, strlen(ADVANCED_HELP_FORMAT_ERROR) + 1, ADVANCED_HELP_FORMAT_ERROR);
	return help_to_show;


HELP_NOMEM_ERROR_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	help_to_show = (char*)malloc(strlen(ADVANCED_HELP_NOMEM_ERROR) + 1);
	if (NULL == help_to_show) {
		return NULL;	// Not even possible to output the error
	}
	strcpy_s(help_to_show, strlen(ADVANCED_HELP_NOMEM_ERROR) + 1, ADVANCED_HELP_NOMEM_ERROR);
	return help_to_show;
}

WCHAR* getAdvancedHelpForKeywordW(_In_ const WCHAR* keyword, _In_ void* help_ptr) {
	WCHAR* help_to_show = NULL;
	WCHAR* full_help_text = NULL;
	WCHAR* current_nodes[MAX_NODE_LEVEL] = { 0 };
	bool current_nodes_already_included[MAX_NODE_LEVEL] = { 0 };
	size_t msg_len = 0;

	if (NULL == help_ptr) {
		goto HELP_UNINITIALIZED_ERROR_LABEL;
	}

	full_help_text = (WCHAR*)malloc(wcslen((WCHAR*)help_ptr) + 1);
	if (NULL == full_help_text) {
		goto HELP_NOMEM_ERROR_LABEL;
	}
	wcscpy_s(full_help_text, wcslen((WCHAR*)help_ptr) + 1, (WCHAR*)help_ptr);

	// Init arrays
	for (size_t i = 0; i < MAX_NODE_LEVEL; i++) {
		current_nodes[i] = NULL;
		current_nodes_already_included[i] = false;
	}

	if (0 == wcscmp(L"", keyword)) {
		return full_help_text;
	}

	// Nodes are the same as lines, but without the NODE_START_CHAR if needed. No memory is allocated for lines nor nodes
	WCHAR* line = NULL;
	WCHAR* current_node = NULL;
	WCHAR* new_node = NULL;
	size_t current_node_level = 0;
	size_t forced_include_min_level = MAX_NODE_LEVEL;

	// Process all the help iteratively line by line
	// Note:
	//    When there are no more tokens, line becomes NULL, new_node becomes NULL too, and it will be different from current_node.
	//    Then the last node is processed.
	//    Finally, current_node is assigned to new_node (=NULL), allowing the loop to exit
	int iteration = 0;
	WCHAR* wcstok_ctx = NULL;
	do {
		iteration++;
		//printf("\n\niteration %d\n", iteration);

		if (NULL == line) {
			line = wcstok_s(full_help_text, L"\n", &wcstok_ctx);
		} else {
			line = wcstok_s(NULL, L"\n", &wcstok_ctx);
		}
		new_node = getLineNodeW(line, current_node);
		//printf("  line (len=%d):\n%ws\n", (NULL == line) ? -1 : (int)wcslen(line), line);
		//printf("  current_node (len=%d):\n%ws\n", (NULL == current_node) ? -1 : (int)wcslen(current_node), current_node);
		//printf("  new_node (len=%d):\n%ws\n", (NULL == new_node) ? -1 : (int)wcslen(new_node), new_node);

		//If current_node is NULL, assign it to the first node and continue, because it is the first iteration
		if (NULL == current_node) {
			current_node = new_node;
			continue;
		}
		// If the node is the same, continue reading because more tokens may fuse into the same node.
		if (new_node == current_node || NULL == current_node) {
			continue;
		}
		//printf("  Proceed to processing...\n");

		// Process current node (already complete with all its lines)
		current_node_level = getNodeLevelW(current_node);
		//printf("  current_node_level=%llu\n", current_node_level);

		// Check that nodes do not skip levels (eg, a level 1 node followed by level 3 node without a level 2 node in between)
		for (size_t i = 0; i < current_node_level; i++) {
			if (NULL == current_nodes[i]) {
				//printf("current_nodes[%llu]=NULL\n", i);
				goto HELP_FORMAT_ERROR_LABEL;
			}
		}

		// Assign current node
		current_nodes[current_node_level] = current_node;
		current_nodes_already_included[current_node_level] = false;

		// Clear subnodes
		for (size_t i = current_node_level + 1; i < MAX_NODE_LEVEL; i++) {
			if (NULL == current_nodes[i]) {
				break;
			}
			current_nodes[i] = NULL;
			current_nodes_already_included[i] = false;
		}

		// Check if this node is forced to be added (due to parent node included the keyword)
		if (forced_include_min_level < current_node_level) {
			if ((0 != wcsAppendRealloc(&help_to_show, current_node)) || (0 != wcsAppendRealloc(&help_to_show, L"\n"))) {
					goto HELP_NOMEM_ERROR_LABEL;
			}
		} else {
			// Stop forcing to include
			forced_include_min_level = MAX_NODE_LEVEL;

			// Search the keyword in the node
			if (NULL != wcsstr(current_node, keyword)) {	// Keyword found!
				// Include parent nodes if not already included (+ 1 takes care of the current node)
				for (size_t i = 0; i < current_node_level + 1; i++) {
					if (!current_nodes_already_included[i]) {
						if ((0 != wcsAppendRealloc(&help_to_show, current_nodes[i])) || (0 != wcsAppendRealloc(&help_to_show, L"\n"))) {
							goto HELP_NOMEM_ERROR_LABEL;
						}
						current_nodes_already_included[i] = true;
					}
				}

				// Include current node (already done in the loop above?)
				//if ((0 != wcsAppendRealloc(&help_to_show, current_node)) || (0 != wcsAppendRealloc(&help_to_show, "\n"))) {
				//	goto HELP_NOMEM_ERROR_LABEL;
				//}
				//current_nodes_already_included[current_node_level] = true;

				// Force include everything below this level
				forced_include_min_level = current_node_level;
			}
		}

		current_node = new_node;
	} while (NULL != current_node);

	// No errors
	if (NULL == help_to_show) {
		goto HELP_KEYWORD_NOT_FOUND_LABEL;
	}
	return help_to_show;

HELP_UNINITIALIZED_ERROR_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	msg_len = wcslen(WTEXT(ADVANCED_HELP_UNINITIALIZED_ERROR));
	help_to_show = (WCHAR*)malloc(sizeof(WCHAR)*(msg_len + 1));
	if (NULL == help_to_show) {
		goto HELP_NOMEM_ERROR_LABEL;
		//return NULL;	// Not even possible to output the error
	}
	wcscpy_s(help_to_show, msg_len + 1, WTEXT(ADVANCED_HELP_UNINITIALIZED_ERROR));
	return help_to_show;


HELP_KEYWORD_NOT_FOUND_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	msg_len = wcslen(WTEXT(ADVANCED_HELP_KEYWORD_NOT_FOUND_INFO));
	help_to_show = (WCHAR*)malloc(sizeof(WCHAR) * (msg_len + 1));
	if (NULL == help_to_show) {
		goto HELP_NOMEM_ERROR_LABEL;
		//return NULL;	// Not even possible to output the error
	}
	wcscpy_s(help_to_show, msg_len + 1, WTEXT(ADVANCED_HELP_KEYWORD_NOT_FOUND_INFO));
	return help_to_show;


HELP_FORMAT_ERROR_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	msg_len = wcslen(WTEXT(ADVANCED_HELP_FORMAT_ERROR));
	help_to_show = (WCHAR*)malloc(sizeof(WCHAR)* (msg_len + 1));
	if (NULL == help_to_show) {
		goto HELP_NOMEM_ERROR_LABEL;
		//return NULL;	// Not even possible to output the error
	}
	wcscpy_s(help_to_show, msg_len + 1, WTEXT(ADVANCED_HELP_FORMAT_ERROR));
	return help_to_show;


HELP_NOMEM_ERROR_LABEL:
	if (NULL != help_to_show) {
		free(help_to_show);
		help_to_show = NULL;
	}
	if (NULL != full_help_text) {
		free(full_help_text);
		full_help_text = NULL;
	}
	// Try to copy the error description as output
	msg_len = wcslen(WTEXT(ADVANCED_HELP_NOMEM_ERROR));
	help_to_show = (WCHAR*)malloc(sizeof(WCHAR) * (msg_len + 1));
	if (NULL == help_to_show) {
		return NULL;	// Not even possible to output the error
	}
	wcscpy_s(help_to_show, msg_len + 1, WTEXT(ADVANCED_HELP_NOMEM_ERROR));
	return help_to_show;
}


char* getLineNode(_In_ char* new_line, _In_opt_ char* current_node) {
	if (NULL == new_line) {
		return NULL;
	}
	if (NULL == current_node) {
		return new_line;
	}

	if ('\0' == NODE_START_CHAR) {
		return new_line;	// New line is always a new node
	} else if (new_line[0] == NODE_START_CHAR) {
		return &(new_line[1]);	// New line is a new node, but special character is removed
	} else {
		// New line is part of the last node. Concatenate restoring removed '\n' (can be several due to strtok behaviour) and return the same node again.
		//current_node[strlen(current_node)] = '\n';
		size_t i = strlen(current_node);
		while ('\0' == current_node[i]) {
			current_node[i] = '\n';
			i++;
		}
		return current_node;
	}
}
WCHAR* getLineNodeW(_In_ WCHAR* new_line, _In_opt_ WCHAR* current_node) {
	if (NULL == new_line) {
		return NULL;
	}
	if (NULL == current_node) {
		return new_line;
	}

	if (L'\0' == WTEXT(NODE_START_CHAR)) {
		return new_line;	// New line is always a new node
	} else if (new_line[0] == WTEXT(NODE_START_CHAR)) {
		return &(new_line[1]);	// New line is a new node, but special character is removed
	} else {
		// New line is part of the last node. Concatenate restoring removed '\n' (can be several due to strtok behaviour) and return the same node again.
		//current_node[wcslen(current_node)] = L'\n';
		size_t i = wcslen(current_node);
		while (L'\0' == current_node[i]) {
			current_node[i] = L'\n';
			i++;
		}
		return current_node;
	}
}

size_t getNodeLevel(_In_ const char* current_node) {
	if (NULL == current_node) {
		return 0;
	}

	size_t current_node_level = 0;
	for (size_t i = 0; i < strlen(current_node); i++) {
		if (NODE_LEVEL_CHAR == current_node[i]) {
			current_node_level++;
		}
	}
	return current_node_level;
}

size_t getNodeLevelW(_In_ const WCHAR* current_node) {
	if (NULL == current_node) {
		return 0;
	}

	size_t current_node_level = 0;
	for (size_t i = 0; i < wcslen(current_node); i++) {
		if (WTEXT(NODE_LEVEL_CHAR) == current_node[i]) {
			current_node_level++;
		}
	}
	return current_node_level;
}


/**
 * @brief Appends a source string (src) to a destination string (dest), dynamically resizing dest's memory using realloc.
 * If *dest is NULL, the function allocates memory and initializes the string with src.
 * The destination pointer is modified directly.
 *
 * @param dest Pointer to a pointer of the destination string (char **).
 * @param src The source string to append (const char *).
 * @return int Returns 0 if the operation was successful, -1 if a realloc error occurred.
 */
int strAppendRealloc(_Inout_ char** dest, _In_ const char* src) {
	// If the source string is NULL or empty, do nothing.
	if (NULL == src || '\0' == *src) {
		return 0; // Success, nothing was appended.
	}

	// Get lengths
	size_t src_len = strlen(src);
	size_t current_len = 0;
	if (NULL  != *dest) {
		current_len = strlen(*dest);
	}

	// Reallocate memory (*dest being NULL is already handled by realloc)
	char* tmp_ptr = (char*)realloc(*dest, sizeof(char) * (current_len + src_len + 1)); //+ 1 for the final '\0'
	if (NULL == tmp_ptr) {
		// Memory allocation failed. Leave the original pointer intact.
		//perror("Error in realloc");
		return -1;
	}
	*dest = tmp_ptr;

	// Start copying in the first available position (current_len is 0 for the first allocation, and the end of the current string otherwise)
	strcpy_s((*dest) + current_len, current_len + src_len + 1, src);

	return 0;
}
/**
 * @brief Appends a source string (src) to a destination string (dest), dynamically resizing dest's memory using realloc.
 * If *dest is NULL, the function allocates memory and initializes the string with src.
 * The destination pointer is modified directly.
 *
 * @param dest Pointer to a pointer of the destination string (char **).
 * @param src The source string to append (const char *).
 * @return int Returns 0 if the operation was successful, -1 if a realloc error occurred.
 */
int wcsAppendRealloc(_Inout_ WCHAR** dest, _In_ const WCHAR* src) {
	// If the source string is NULL or empty, do nothing.
	if (NULL == src || L'\0' == *src) {
		return 0; // Success, nothing was appended.
	}

	// Get lengths
	size_t src_len = wcslen(src);
	size_t current_len = 0;
	if (NULL != *dest) {
		current_len = wcslen(*dest);
	}

	// Reallocate memory (*dest being NULL is already handled by realloc)
	WCHAR* tmp_ptr = (WCHAR*)realloc(*dest, sizeof(WCHAR) * (current_len + src_len + 1)); //+ 1 for the final '\0'
	if (NULL == tmp_ptr) {
		// Memory allocation failed. Leave the original pointer intact.
		//perror("Error in realloc");
		return -1;
	}
	*dest = tmp_ptr;

	// Start copying in the first available position (current_len is 0 for the first allocation, and the end of the current string otherwise)
	wcscpy_s((*dest) + current_len, current_len + src_len + 1, src);

	return 0;
}

void freeAdvancedHelp(_In_ void** help_ptr) {
	if (NULL != *help_ptr) {
		free(*help_ptr);
		*help_ptr = NULL;
	}
	return;
}
void freeAdvancedHelpW(_In_ void** help_ptr) {
	freeAdvancedHelp(help_ptr);
}

int initAdvancedHelp(_In_ const char* help_filename, _Inout_ void** help_ptr) {
	return getTextFromFile(help_filename, (char**)help_ptr);
//	// Check if already initialized
//	if (NULL != *help_ptr) {
//		return -1;
//	}
//
//	FILE* fp = NULL;
//	errno_t error = 0;
//	error = fopen_s(&fp, help_filename, "r");
//	if (NULL == fp) {
//		error = -1;
//		goto INIT_HELP_ERROR_LABEL;
//	}
//	// Go to EOF and get file_size
//	if (fseek(fp, 0L, SEEK_END) == 0) {
//		long file_size = ftell(fp);
//		if (file_size == -1 || file_size < 0) {
//			error = -2;
//			goto INIT_HELP_ERROR_LABEL;
//		}
//
//		// Allocate buffer
//		*help_ptr = malloc(sizeof(char) * ((size_t)file_size + 1));
//		if (NULL == *help_ptr) {
//			error = -2;
//			goto INIT_HELP_ERROR_LABEL;
//		}
//
//		// Reset file pointer to the start of the file
//		if (fseek(fp, 0L, SEEK_SET) != 0) {
//			error = -3;
//			goto INIT_HELP_ERROR_LABEL;
//		}
//
//		// Read the entire file into memory
//		size_t read_len = fread(*help_ptr, sizeof(char), file_size, fp);
//		if (ferror(fp) != 0) {
//			//fputs("Error reading file", stderr);
//			error = -4;
//			goto INIT_HELP_ERROR_LABEL;
//		} else {
//			((char*)(*help_ptr))[read_len++] = '\0';
//		}
//	}
//	fclose(fp);
//
//	return 0;
//
//INIT_HELP_ERROR_LABEL:
//	if (NULL != fp) {
//		fclose(fp);
//		fp = NULL;
//	}
//	if (NULL != *help_ptr) {
//		free(*help_ptr);
//		*help_ptr = NULL;
//	}
//	return error;
}
int initAdvancedHelpW(_In_ const WCHAR* help_filename, _Inout_ void** help_ptr) {
	return getTextFromFileW(help_filename, (WCHAR**)help_ptr);
}

int getTextFromFile(_In_ const char* text_filename, _Inout_ char** text_ptr) {
	// Check if already initialized
	if (NULL != *text_ptr) {
		return -1;
	}
	if (NULL == text_filename) {
		return -1;
	}

	FILE* fp = NULL;
	errno_t error = 0;
	error = fopen_s(&fp, text_filename, "r");
	if (NULL == fp) {
		// The error whatever the file opening function says
		goto GET_TEXT_ERROR_LABEL;
	}
	// Move pointer to EOF, get the file size, and rewind the pointer to the start of the file
	if (fseek(fp, 0L, SEEK_END) == 0) {
		long file_size = ftell(fp);
		if (file_size == -1 || file_size < 0) {
			error = -2;
			goto GET_TEXT_ERROR_LABEL;
		}
		rewind(fp);

		// Allocate buffer
		size_t buf_size = sizeof(char) * ((size_t)file_size + 1);
		*text_ptr = malloc(buf_size);
		if (NULL == *text_ptr) {
			error = -2;
			goto GET_TEXT_ERROR_LABEL;
		}

		// Read the entire file into memory
		size_t read_len = fread_s(*text_ptr, buf_size, sizeof(char), file_size, fp);
		if (ferror(fp) != 0) {
			//fputs("Error reading file", stderr);
			error = -4;
			goto GET_TEXT_ERROR_LABEL;
		} else {
			(*text_ptr)[read_len] = '\0';
		}
	}
	fclose(fp);

	return 0;

GET_TEXT_ERROR_LABEL:
	if (NULL != fp) {
		fclose(fp);
		fp = NULL;
	}
	if (NULL != *text_ptr) {
		free(*text_ptr);
		*text_ptr = NULL;
	}
	return error;
}
int getTextFromFileW(_In_ const WCHAR* text_filename, _Inout_ WCHAR** text_ptr) {
	// Check if already initialized
	if (NULL != *text_ptr) {
		return -1;
	}
	if (NULL == text_filename) {
		return -1;
	}

	FILE* fp = NULL;
	FILE* fp2 = NULL;
	errno_t error = 0;
	error = _wfopen_s(&fp, text_filename, L"r, ccs=UTF-8");
	//error = _wfopen_s(&fp, text_filename, L"rb");
	if (NULL == fp) {
		// The error whatever the file opening function says
		goto GET_TEXT_W_ERROR_LABEL;
	}
	// Move pointer to EOF, get the file size, and rewind the pointer to the start of the file
	if (fseek(fp, 0L, SEEK_END) == 0) {
		long file_size = ftell(fp);
		if (file_size == -1 || file_size < 0) {
			error = -2;
			goto GET_TEXT_W_ERROR_LABEL;
		}
		rewind(fp);

		// Allocate buffer
		size_t buf_size = sizeof(WCHAR) * ((size_t)file_size + 1);
		//size_t buf_size = file_size + sizeof(WCHAR);
		*text_ptr = malloc(buf_size);
		if (NULL == *text_ptr) {
			error = -2;
			goto GET_TEXT_W_ERROR_LABEL;
		}

		// Read the entire file into memory
		size_t read_len = fread_s(*text_ptr, buf_size, sizeof(WCHAR), file_size, fp);
		//size_t read_len = fread_s((void*)(*text_ptr), buf_size, 1, file_size, fp);
		if (ferror(fp) != 0/* || read_len != file_size*/) {
			//fputs("Error reading file", stderr);
			printf("read_len = %llu, file_size = %lu\n", read_len, file_size);
			error = -4;
			goto GET_TEXT_W_ERROR_LABEL;
		} else {
			printf("read_len = %llu, file_size = %lu, buf_size = %llu\n", read_len, file_size, buf_size);
			(*text_ptr)[read_len] = L'\0';
			//(*text_ptr)[read_len / sizeof(wchar_t)] = L'\0';
			printf("bine");
		}
	}
	fclose(fp);

	return 0;

GET_TEXT_W_ERROR_LABEL:
	if (NULL != fp) {
		fclose(fp);
		fp = NULL;
	}
	if (NULL != *text_ptr) {
		free(*text_ptr);
		*text_ptr = NULL;
	}
	return error;
}

