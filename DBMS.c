/*
Yingjie(Chelsea) Wang
CMPSC 431 
Professor: Steven Shaffer
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXINPUTLENGTH 5000
#define MAXLENOFFIELDNAMES 50
#define MAXLENOFFIELDTYPES 50
int xmain();

typedef struct _field {
	char fieldName[MAXLENOFFIELDNAMES];
 	char fieldType[MAXLENOFFIELDTYPES];
 	int fieldLength;
} field_t;

typedef struct _condition {
   int field_index, field_other;
   bool literal;
   char *match;
   struct _condition *next;
} condition_t;

typedef struct _table {
   char *name;
   struct _table *next;
} table_t;

typedef table_t field_ref_t;

//prototype avoid of warning: implicit declaration of function
void createTable(char *name, char *buffer);
void createIndex(char *name, char *buffer);
void insert(char *name, char *buffer);
void selectVar(char *buffer);
void dropTable(char *buffer);

void trimwhitespace(char *buf) {
   char *begin = buf, *end = buf + strlen(buf);
   
   // Add a null terminator at the buffer
   while (end > buf && *end == '\0' || *end == '\n' || *end == '\r' || *end == ' ' || *end == '\t') {
      *end = '\0';
      end--;
   }

}

void processCommand(char *buffer) {
    // breaks string str into a series of tokens using /n.
    char *token = strtok(buffer, " \n");
    if (token == NULL) {
        printf("No input received.");
        exit(1); 
    } else if (strncmp(token, "CREATE", MAXINPUTLENGTH) == 0) {
        token = strtok(NULL, " \n");
        if (token != NULL && strncmp(token, "TABLE", MAXINPUTLENGTH) == 0) {
            token = strtok(NULL, " \n");
            createTable(token, buffer);
            return;
        } else if (strcmp(token, "INDEX") == 0) {
      	 token = strtok(NULL, " \n");
         createIndex(token, buffer);
        } else {
            printf("Wrong create statement.");
            exit(1);  
        }
    } else if (strncmp(token, "INSERT", MAXINPUTLENGTH) == 0) {
        token = strtok(NULL, " \n");
        if (token != NULL && strncmp(token, "INTO", MAXINPUTLENGTH) == 0) {      
      	    token = strtok(NULL, " \n");
            insert(token, buffer);
            return;
        } else {
            printf("Wrong insert statement.");
            exit(1);    
        }
    } else if (strncmp(token, "SELECT", MAXINPUTLENGTH) == 0) {
        selectVar(buffer);
    } else if (strncmp(token, "DROP", MAXINPUTLENGTH) == 0) {
        dropTable(buffer);
    }else {
        printf("Nothing is not called");
        exit(1);
    }
}


void createTable(char *name, char *buffer) {
    
    if (name == NULL) {
        printf("no table name");
        exit(2);
    }
    
    // get the file name and create bin file
    char *binName = malloc(strlen(name) + 5);
    char *extension=".bin";
    strcpy(binName, name);
    strcat(binName, extension);
    FILE *binFile = fopen(binName, "wb");
    if (binFile == NULL) {
        printf("Failed to open file %s", binName);
        exit(3);
    }
    
    // get the file name and create schema file
    char *schemaName = malloc(strlen(name) + 8);
    char *extension1=".schema";
    strcpy(schemaName, name);
    strcat(schemaName, extension1);
    FILE *schemaFile = fopen(schemaName, "wb");
    if (schemaFile == NULL) {
        printf("Failed to open file %s", schemaName);
        exit(3);
    }
    
    
    // create a struct to store our field info in temporarily
    field_t tmp;
        
    while (fgets(buffer, MAXINPUTLENGTH - 1, stdin)) {     
        printf("===> %s", buffer);
        // add new fields
        char *token = strtok(buffer, " \n");
        if (strncmp(token, "ADD", MAXINPUTLENGTH) == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {     
                // copy field name to struct
                strncpy(tmp.fieldName, token, MAXLENOFFIELDNAMES);
                token = strtok(NULL, " ");
                if (token != NULL) {
                    // copy field type to struct
                    strncpy(tmp.fieldType, token, MAXLENOFFIELDTYPES);
                    token = strtok(NULL, " \n");
                    if (token != NULL) {
                        int len = atoi(token);
                        if (len > 0) {
                            // add the length of the field to struct
                            tmp.fieldLength = len;
                            // write the field to the schema file
                            fwrite(&tmp, sizeof(field_t), 1, schemaFile);
                        } else {
                            exit(2);  
                        }
                    } else {
                        exit(2);
                    }
                } else {
                    exit(2);
                }
            } else {
             exit(2);
          }
        } else if (strncmp(token, "END", MAXINPUTLENGTH) == 0) {
            break;   
        } else {
            exit(2);    
        }
    }
    
    //closer files and free up space
    fclose(binFile);
    fclose(schemaFile);
    free(schemaName);
    free(binName);
}

void createIndex(char *name, char *buffer) {
   if (name == NULL) {
      printf("no index name");
      exit(6);
   }
   
   char *index_name = malloc(strlen(name) + 1);
   strcpy(index_name, name);
   
   char *token = strtok(NULL, " \n");
   if (token == NULL || strcmp(token, "USING")) {
      printf("missing USING");
      exit(6);
   }
   
   // get field name(s)
   field_ref_t *fields = NULL;
   field_ref_t *last = NULL;
   int field_count = 0;
   while (token = strtok(NULL, " ,\n")) {
      if (last == NULL) {
         last = malloc(sizeof(field_ref_t));
         fields = last;
      } else {
         last->next = malloc(sizeof(field_ref_t));
         last = last->next;
      }
      last->name = malloc(strlen(token) + 1);
      strcpy(last->name, token);
      last->next = NULL;
      field_count++;
   }
   
   fgets(buffer, MAXINPUTLENGTH - 1, stdin);
   printf("===> %s", buffer);
   token = strtok(buffer, " \n");
   
   if (token == NULL || strcmp(token, "FROM")) {
      printf("missing FROM");
      exit(6);
   }
    
   // get table(s) for index
   table_t *tables = NULL;
   last = NULL;
   while (token = strtok(NULL, " \n")) {
      if (last == NULL) {
         last = malloc(sizeof(table_t));
         tables = last;
      } else {
         last->next = malloc(sizeof(table_t));
         last = last->next;
      }
      last->name = malloc(strlen(token) + 1);
      strcpy(last->name, token);
      last->next = NULL;
   }
   
   fgets(buffer, MAXINPUTLENGTH - 1, stdin);
   token = strtok(buffer, " \n");
   printf("===> %s\n", buffer);
   
   if (token == NULL || strcmp(token, "END")) {
      printf("missing END");
      exit(6);
   }
   
   // asume only one table for now
   field_t *index_fields = calloc(field_count, sizeof(field_t));
   int *field_offsets = malloc(field_count * sizeof(int));
   int *new_offsets = malloc(field_count * sizeof(int));
   char *schema_file = malloc(strlen(tables->name) + 8);
   strcpy(schema_file, tables->name);
   strcat(schema_file, ".schema");
   FILE *tbl_schema = fopen(schema_file, "r");
   free(schema_file);
   
   // get the schema entry and offset (in table) for each field
   field_t field_buf;
   int field_num;
   int field_offset = 0;
   int new_offset = 0;
   while (fread(&field_buf, sizeof(field_t), 1, tbl_schema) == 1) {
      field_ref_t *tmp = fields;
      field_num = 0;
      while (tmp != NULL) {
         if (strcmp(tmp->name, field_buf.fieldName) == 0) {
            memcpy(index_fields + field_num, &field_buf, sizeof(field_t));
            field_offsets[field_num] = field_offset;
            new_offsets[field_num] = new_offset;
            new_offset += field_buf.fieldLength;
            break;
         }
         
         tmp = tmp->next;
         field_num++;
      }
      field_offset += field_buf.fieldLength;
   }
   
   fclose(tbl_schema);
   
   char *index_schema = malloc(strlen(index_name) + 7);
   strcpy(index_schema, index_name);
   strcat(index_schema, ".index");
   
   // Write index fields to schema
   FILE *index_file = fopen(index_schema, "w");
   free(index_schema);
   fwrite(index_fields, sizeof(field_t), field_count, index_file);
   fclose(index_file);
   
   // Copy specified fields from table bin to index bin
   char *bin_file = malloc(strlen(tables->name) + 5);
   strcpy(bin_file, tables->name);
   strcat(bin_file, ".bin");
   
   char *index_bin = malloc(strlen(index_name) + 7);
   strcpy(index_bin, index_name);
   strcat(index_bin, "_i.bin");
   
   
   FILE *tbl_bin = fopen(bin_file, "r");
   char *tmpfile = tmpnam(NULL);
   index_file = fopen(tmpfile, "w");
   
   
   free(bin_file);
   
   char *record = malloc(field_offset); // Field offset is the size of a row
   
   while (fread(record, field_offset, 1, tbl_bin)) {
      for (int i = 0; i < field_count; i++) {
         fwrite(record + field_offsets[i], strlen(record + field_offsets[i]), 1, index_file);
         if (i != field_count - 1) {
            fwrite(",", 1, 1, index_file);
         }
      }
      fwrite("\n", 1, 1, index_file);
   }
   
   // Clean up space
   free(field_offsets);
   free(index_fields);
   free(record);
   fclose(tbl_bin);
   fclose(index_file);
   
   // Sort the file
   char cmd[2 * MAXLENOFFIELDNAMES + 20];
   sprintf(cmd, "sort %s > %s.srt", tmpfile, index_bin);
   system(cmd);
   
   // Back to binary
   char *sorted_file = malloc(strlen(index_bin) + 5);
   strcpy(sorted_file, index_bin);
   strcat(sorted_file, ".srt");
   
   index_file = fopen(sorted_file, "r");
   FILE *sorted_bin = fopen(index_bin, "w");
   
   record = malloc(new_offset);
   while (fgets(buffer, MAXINPUTLENGTH - 1, index_file) && strlen(buffer) > 4) {
      char *entry = strtok(buffer, ",\n");
      int field = 0;
      while (entry != NULL) {
         strcpy(record + new_offsets[field], entry);
         field++;
         entry = strtok(NULL, ",\n");
      }
      fwrite(record, new_offset, 1, sorted_bin);
   }
   
   free(index_bin);
   free(record);
   fclose(index_file);
   fclose(sorted_bin);
}


void insert(char *name, char *buffer) {
    
    // open our bin file to append new records
    char *binName = malloc(strlen(name) + 5);
    char *extension=".bin";
    strcpy(binName, name);
    strcat(binName, extension);
    FILE *binFile = fopen(binName, "a");
    if (binFile == NULL) {
        printf("Failed to open file %s", binName);
        exit(3);
    }
    
    char *schemaName = malloc(strlen(name) + 8);
    char *extension1=".schema";
    strcpy(schemaName, name);
    strcat(schemaName, extension1);
    FILE *schemaFile = fopen(schemaName, "rb");
    if (schemaFile == NULL) {
        printf("Failed to open file %s", schemaName);
        exit(3);
    }
      
    // get the number of fields from the size of the file
    struct stat info;
    fstat(fileno(schemaFile), &info);
    int field_count = info.st_size / sizeof(field_t);
   
    int record_size = 0;
    field_t *fields = malloc(info.st_size);
    for (int i = 0; i < field_count; i++) {
        fread(fields + i, 1, sizeof(field_t), schemaFile);
        record_size += fields[i].fieldLength;
    }
    
    void *tmp_rec = malloc(record_size);
    
    int offset;
    
    char *token = strtok(NULL, ",\n");
    for (int i = 0; i < field_count; i++) {
        if (token == NULL) {
            exit(4); 
        }
        
        // deal with char types
        if (strncmp(fields[i].fieldType, "char", 5) == 0) {
            if (strlen(token) >= fields[i].fieldLength) {
                printf("*** WARNING: Data in field %s is being truncated ***\n", fields[i].fieldName);
                
            }
            
            strncpy(tmp_rec + offset, token, fields[i].fieldLength - 1);
            trimwhitespace(tmp_rec + offset);
         } else {
            exit(4);
         }

         offset += fields[i].fieldLength;
         token = strtok(NULL, ",\n");
    }
    fwrite(tmp_rec, 1, record_size, binFile);
    
    // free up space
    fclose(binFile);   
    free(binName);
    free(schemaName);
    free(fields);
    free(tmp_rec);
}

int is_operator(char *field) {
    return strcmp(field, "+") == 0 || strcmp(field, "-") == 0 || strcmp(field, "*") == 0 || strcmp(field, "/") == 0;
}

float eval(char *value1, char *operator, char *value2) {
    float f1 = atof(value1);
    float f2 = atof(value2);
    
    if (strcmp(operator, "+") == 0)
        return f1 + f2;
    else if (strcmp(operator, "-") == 0)
        return f1 - f2;
    else if (strcmp(operator, "*") == 0)
        return f1 * f2;
    else if (strcmp(operator, "/") == 0)
        return f1 / f2;
    else {
        printf ("ERROR: unrecognized operator (%s)", operator);
        return 0;
    }
}

void selectVar(char *buffer) {
   char *token = strtok(NULL, " ,\n");
   
   int field_buffer_max = 50;
   int field_count = 0;
   char **fields = malloc(field_buffer_max * sizeof(char*));
   
   // read fields until end of line
   while (token != NULL) {
      // Dynamically resize array if full
      if (field_count == field_buffer_max) {
         field_buffer_max += 10;
         fields = realloc(fields, field_buffer_max * sizeof(char*));
      }
      fields[field_count] = malloc(strlen(token) + 1);
      strcpy(fields[field_count++], token);
      token = strtok(NULL, " ,\n");
   }
   fgets(buffer, MAXINPUTLENGTH - 1, stdin);
   printf("===> %s", buffer);
   token = strtok(buffer, " \n");
   
   if (token == NULL || strcmp(token, "FROM")) {
      exit(5);
   }
   
   table_t *tables = NULL;
   table_t *last_table = NULL;
   char *table = strtok(NULL, ", \n");
   while (table != NULL) {
      if (tables == NULL) {
         tables = malloc(sizeof(table_t));
         tables->name = table;
         last_table = tables;
      } else {
         last_table->next = malloc(sizeof(table_t));
         last_table = last_table->next;
         last_table->name = table;
      }
      last_table->next = NULL;
      table = strtok(NULL, ", \n");
   }
   
   if (tables == NULL) {
	  printf("empty FROM");
      exit(5);
   }
    // Join tables in temporary file
   while (tables->next != NULL) {
      // Assign temporary file a name
      char *tmp_file = malloc(strlen(tables->name) + strlen(tables->next->name) + 3);
      strcpy(tmp_file, "_");
      strcat(tmp_file, tables->name);
      strcat(tmp_file, "_");
      strcat(tmp_file, tables->next->name);
      
      // Filename for temporary schema 
      char *fn_schema_tmp = malloc(strlen(tmp_file) + 8);
      strcpy(fn_schema_tmp, tmp_file);
      strcat(fn_schema_tmp, ".schema");
      
      // Filename for temporary bin
      char *fn_bin_tmp = malloc(strlen(tmp_file) + 5);
      strcpy(fn_bin_tmp, tmp_file);
      strcat(fn_bin_tmp, ".bin");
      
      // Concatenate schemas, join bins
      FILE *schema_tmp = fopen(fn_schema_tmp, "w");
      FILE *bin_tmp = fopen(fn_bin_tmp, "w");
      
      // Free up space
      free(fn_schema_tmp);
      free(fn_bin_tmp);
      
      // Get first schema file name
      char *schema_file_first = malloc(strlen(tables->name) + 8);
      strcpy(schema_file_first, tables->name);
      strcat(schema_file_first, ".schema");
      
      // Get second schema file name
      char *schema_file_second = malloc(strlen(tables->next->name) + 8);
      strcpy(schema_file_second, tables->next->name);
      strcat(schema_file_second, ".schema");
      
      // Get first bin file name
      char *bin_file_first = malloc(strlen(tables->name) + 5);
      strcpy(bin_file_first, tables->name);
      strcat(bin_file_first, ".bin");
      
      // Get second bin file name
      char *bin_file_second = malloc(strlen(tables->next->name) + 5);
      strcpy(bin_file_second, tables->next->name);
      strcat(bin_file_second, ".bin");
      
      // Read the first schema and write to tmp
      FILE *schema = fopen(schema_file_first, "r");
      free(schema_file_first);
         
      struct stat info;
      fstat(fileno(schema), &info);
      int row_size[] = {0, 0};
      int field_count = info.st_size / sizeof(field_t);
      
      // Write the schema contents to tmp file
      field_t buffer;
      for (int i = 0; i < field_count; i++) {
         fread(&buffer, 1, sizeof(field_t), schema);
         row_size[0] += buffer.fieldLength;
         fwrite(&buffer, 1, sizeof(field_t), schema_tmp);
      }
      
      // Close the first schema and open the second
      fclose(schema);
      schema = fopen(schema_file_second, "r");
      free(schema_file_second);
      
      // Read the second schema and write to tmp
      fstat(fileno(schema), &info);
      field_count = info.st_size / sizeof(field_t);
      
      for (int i = 0; i < field_count; i++) {
         fread(&buffer, 1, sizeof(field_t), schema);
         row_size[1] += buffer.fieldLength;
         fwrite(&buffer, 1, sizeof(field_t), schema_tmp);
      }
      
      fclose(schema);
      fclose(schema_tmp);
      
      // Join the bin files
      FILE *bin_first = fopen(bin_file_first, "r");
      FILE *bin_second = fopen(bin_file_second, "r");
      free(bin_file_first);
      free(bin_file_second);
      
      int rows[] = {0, 0};
      fstat(fileno(bin_first), &info);
      rows[0] = info.st_size / row_size[0];
      fstat(fileno(bin_second), &info);
      rows[1] = info.st_size / row_size[1];
      
      char *buffer_first = malloc(row_size[0]);
      char *buffer_second = malloc(row_size[1]);
      for (int i = 0; i < rows[0]; i++) {
         fread(buffer_first, 1, row_size[0], bin_first);
         // Return to beginning of second file
         fseek(bin_second, 0, SEEK_SET);
         for (int j = 0; j < rows[1]; j++) {
            fread(buffer_second, 1, row_size[1], bin_second);
            fwrite(buffer_first, 1, row_size[0], bin_tmp);
            fwrite(buffer_second, 1, row_size[1], bin_tmp);
         }
      }
      
      free(buffer_first);
      free(buffer_second);
      fclose(bin_first);
      fclose(bin_second);
      fclose(bin_tmp);
      
      tables->name = tmp_file;
      
      table_t *old = tables->next;
      tables->next = old->next;
      free(old);
      
   }
   
   // Open the files associated with the table
   char *schema_file = malloc(strlen(tables->name) + 8);
   strcpy(schema_file, tables->name);
   strcat(schema_file, ".schema");
   
   char *bin_file = malloc(strlen(tables->name) + 5);
   strcpy(bin_file, tables->name);
   strcat(bin_file, ".bin");
   
   FILE *schema = fopen(schema_file, "r");
   FILE *bin = fopen(bin_file, "r");
    
   // Assume index is the only table in where clause for now
   bool is_index = false;
   if (schema == NULL || bin == NULL) {
      
      
      strcpy(schema_file, tables->name);
   	  strcat(schema_file, ".index");
             
      strcpy(bin_file, tables->name);
   	  strcat(bin_file, "_i.bin");
             
      schema = fopen(schema_file, "r");
      bin = fopen(bin_file, "r");
       
   if (schema == NULL || bin == NULL) {
      
      while (fgets(buffer, MAXINPUTLENGTH - 1, stdin) && strncmp(buffer, "END", 3)) {
         printf("===> %s", buffer);
      }
      printf("===> %s", buffer);
      printf("Table %s does not exist.\n", tables->name);
      return;
   }
       is_index = true;
      
   }
    
   // Set to -1 to determine if fields missing after loading schema
   int *offsets = malloc(field_count * sizeof(int));
   memset(offsets, -1, field_count * sizeof(int));
    
   // Get the number of fields from the size of the file
   struct stat info;
   fstat(fileno(schema), &info);
   int table_field_count = info.st_size / sizeof(field_t);
   char **table_fields = malloc(table_field_count * sizeof(char*));
   int *table_offsets = malloc(table_field_count * sizeof(int));
   
   int offset_cur = 0;
   field_t tmp;
   
   for (int i = 0; i < table_field_count; i++) {
      fread(&tmp, 1, sizeof(field_t), schema);
      
      table_fields[i] = malloc(strlen(tmp.fieldName) + 1);
      strcpy(table_fields[i], tmp.fieldName);
      table_offsets[i] = offset_cur;
      
      // If the field is one we are looking for, record the offset
      for (int j = 0; j < field_count; j++) {
         if (strcmp(fields[j], tmp.fieldName) == 0) {
            offsets[j] = offset_cur;
            break;
         }
      }
      
      offset_cur += tmp.fieldLength;
   }
    
   // Make sure all fields exist
   for (int i = 0; i < field_count; i++) {
      if (offsets[i] == -1 && is_operator(fields[i]) != 1) {
         exit(5);
      }
   }
   
   fclose(schema);
   free(schema_file);
   
   fgets(buffer, MAXINPUTLENGTH - 1, stdin);
   printf("===> %s", buffer);
   token = strtok(buffer, " \n");
   
   bool has_where_clause = false;
   condition_t *conditions = NULL;
   condition_t *last_condition = NULL;
   char *match_tmp;
   if (strcmp(token, "WHERE") == 0) {
      has_where_clause = true;
      
      do {
         token = strtok(NULL, " \n");

         if (token == NULL) {
            printf("where clause error");
            exit(5);
         }
         
         for (int i = 0; i < table_field_count; i++) {
            if (strcmp(token, table_fields[i]) == 0) {
               match_tmp = strtok(NULL, " =\n");
               if (match_tmp[0] == '"') {
                  match_tmp = strtok(match_tmp, "\"");
                  if (conditions == NULL) {
                     conditions = malloc(sizeof(condition_t));
                     conditions->literal = true;
                     conditions->field_index = i;
                     conditions->match = malloc(strlen(match_tmp) + 1);
                     strcpy(conditions->match, match_tmp);
                     last_condition = conditions;
                  } else {
                     last_condition->next = malloc(sizeof(condition_t));
                     last_condition = last_condition->next;
                     last_condition->literal = true;
                     last_condition->field_index = i;
                     last_condition->match = malloc(strlen(match_tmp) + 1);
                     strcpy(last_condition->match, match_tmp);
                  }
                  last_condition->next = NULL;
               break;
               } else {
                  for (int j = 0; j < table_field_count; j++) {
                     if (strcmp(match_tmp, table_fields[j]) == 0) {
                        if (conditions == NULL) {
                           conditions = malloc(sizeof(condition_t));
                           conditions->literal = false;
                           conditions->field_index = i;
                           conditions->field_other = j;
                           last_condition = conditions;
                        } else {
                           last_condition->next = malloc(sizeof(condition_t));
                           last_condition = last_condition->next;
                           last_condition->literal = false;
                           last_condition->field_index = i;
                           last_condition->field_other = j;
                        }
                        last_condition->next = NULL;
                        break;
                     }
                  }
               }
            }
         }
         
         fgets(buffer, MAXINPUTLENGTH - 1, stdin);
         printf("===> %s", buffer);
         token = strtok(buffer, " \n");
         
      } while (strcmp(token, "AND") == 0);
      
      if (strcmp(token, "END")) {
         printf("need end");
         exit(5);
      }
      
   } else if (strcmp(token, "END")) {
      exit(5);
   }
   
// Get number of existing records from file size and final offset
   fstat(fileno(bin), &info);
   int record_count = info.st_size / offset_cur;
   
   char *record_buffer = malloc(offset_cur);

   
   // Use binary search on indicies for first field with 1 match
   if (is_index && has_where_clause) {
      int i = 0, j = record_count - 1;
      while (i < j) {
      	 fseek(bin, (i + j) / 2 * offset_cur, SEEK_SET);
         fread(record_buffer, offset_cur, 1, bin);
         
         // Assume condition is first in index (for sort to be useful)

         int cmp = strcmp(conditions->match, record_buffer);
         
         printf("TRACE: ");
         for (int j = 0; j < field_count - 1; j++) {
            printf("%s,", record_buffer + offsets[j]);
         }
         puts(record_buffer + offsets[field_count - 1]);
         
         if (cmp > 0) {
            i = (i + j) / 2 + 1;
         } else if (cmp < 0) {
            j = (i + j) / 2 - 1;
         } else {
            for (int j = 0; j < field_count - 1; j++) {
               printf("%s,", record_buffer + offsets[j]);
            }
            puts(record_buffer + offsets[field_count - 1]);
            return;
         }

      }
      
   } else {
      for (int i = 0; i < record_count; i++) {
         fread(record_buffer, offset_cur, 1, bin);
         if (has_where_clause) { 
            condition_t *tmp = conditions;
            bool match = true;
            while (tmp != NULL) {
               int field = tmp->field_index;
               if (tmp->literal) {
                  if (strcmp(tmp->match, record_buffer + table_offsets[tmp->field_index])) {
                     match = false;
                     break;
                  }
               } else {
                  if (strcmp(record_buffer + table_offsets[tmp->field_other], 
                                 record_buffer + table_offsets[tmp->field_index])) {
                     match = false;
                     break;
                  }
               }

               tmp = tmp->next;
            }

            if (!match)
               continue; 
         }
         for (int j = 0; j < field_count - 1; j++) {
            printf("%15s", record_buffer + offsets[j]);////////////////////////////
         }
	printf("      ");
         puts(record_buffer + offsets[field_count-1]);
      }
   }
   // Close and free all the things
   fclose(bin);
   free(bin_file);
   free(record_buffer);
   free(offsets);
   free(fields);
}

// need more implement in the future
void dropTable(char *buffer) {
   char *token = strtok(NULL, " \n");
   if (token == NULL || strcmp(token, "TABLE")) {
      exit(6);
   } else {
      token = strtok(NULL, " \n");
      char *fname = malloc(strlen(token) + 8);
      strcpy(fname, token);
      strcat(fname, ".schema");
      remove(fname);
      strcpy(fname, token);
      strcat(fname, ".bin");
      remove(fname);
   }
}


int main() {
	static char buffer[MAXINPUTLENGTH];
    //copies the character 0 (an unsigned char) to the first n characters of the string pointed to buffer.
    //Initial the buffer with 0
	memset(buffer, 0, MAXINPUTLENGTH);
    //read in input1.txt
 	char *status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
	while (status != NULL) {
        //deal with whitespace issue
     	trimwhitespace(buffer);
     	if (strlen(buffer) < 5)
         	break; // not a real command, CR/LF, extra line, etc.
     	printf("===> %s\n", buffer);
		processCommand(buffer);
 		status = fgets(buffer, MAXINPUTLENGTH-1, stdin); 
    }
 	return 0;
}


