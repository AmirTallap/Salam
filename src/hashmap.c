#include "hashmap.h"

/**
 * 
 * @function hash_function
 * @brief Hash function
 * @params {const char*} key
 * @returns {unsigned long}
 * 
 */
unsigned long hash_function(const char* str)
{
    DEBUG_ME;
	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

/**
 * 
 * @function hashmap_create
 * @brief Create a new hashmap
 * @params {size_t} size
 * @returns {hashmap_t*}
 * 
 */
hashmap_t* hashmap_create(size_t capacity)
{
    DEBUG_ME;
	hashmap_t* map = memory_allocate(sizeof(hashmap_t));

	map->capacity = capacity;
	map->length = 0;
	map->data = (hashmap_entry_t**) memory_callocate(map->capacity, sizeof(hashmap_entry_t*));

	map->print = cast(void (*)(void*), hashmap_print);
	map->destroy = cast(void (*)(void*), hashmap_destroy);

	return map;
}

/**
 * 
 * @function hashmap_put
 * @brief Put a key-value pair in the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @params {void*} value
 * @returns {void}
 * 
 */
void hashmap_put(hashmap_t* map, const char* key, void* value)
{
    DEBUG_ME;
	hashmap_put_custom(map, key, value, free);
}

/**
 * 
 * @function hashmap_put_custom
 * @brief Put a key-value pair in the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @params {void*} value
 * @params {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_put_custom(hashmap_t* map, const char* key, void* value, void (*free_fn)(void*))
{
    DEBUG_ME;
	unsigned long hash = hash_function(key);

	size_t index = hash % map->capacity;
	hashmap_entry_t* entry = map->data[index];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (free_fn != NULL) {
				free_fn(entry->value);
			}

			entry->value = value;

			return;
		}

		entry = cast(hashmap_entry_t*, entry->next);
	}

	hashmap_entry_t* new_entry = memory_allocate(sizeof(hashmap_entry_t));

	new_entry->key = strdup(key);
	new_entry->value = value;
	new_entry->next = cast(struct hashmap_entry_t*, map->data[index]);

	map->data[index] = new_entry;

	map->length++;

	if ((float)map->length / map->capacity >= 0.75) {
		size_t new_length = map->capacity * 2;
		hashmap_entry_t **new_data = (hashmap_entry_t**) memory_callocate(new_length, sizeof(hashmap_entry_t*));

		for (size_t i = 0; i < map->capacity; i++) {
			hashmap_entry_t* entry = map->data[i];

			while (entry) {
				hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);
				unsigned long new_index = hash_function(entry->key) % new_length;

				entry->next = cast(struct hashmap_entry_t*, new_data[new_index]);
				new_data[new_index] = entry;
				entry = next;
			}
		}

		if (map->data != NULL) {
			memory_destroy(map->data);
		}

		map->data = new_data;
		map->capacity = new_length;
	}
}

/**
 * 
 * @function hashmap_get
 * @brief Get a value from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 * 
 */
void* hashmap_get(hashmap_t* map, const char* key)
{
    DEBUG_ME;
	unsigned long hash = hash_function(key);
	size_t index = hash % map->capacity;
	hashmap_entry_t* entry = map->data[index];
	
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return entry->value;
		}

		entry = cast(hashmap_entry_t*, entry->next);
	}

	return NULL;
}

/**
 * 
 * @function hashmap_has
 * @brief Check if the hashmap has a key
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {bool}
 * 
 */
bool hashmap_has(hashmap_t* map, const char* key)
{
	DEBUG_ME;
	unsigned long hash = hash_function(key);
	size_t index = hash % map->capacity;
	hashmap_entry_t* entry = map->data[index];
	
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return true;
		}

		entry = cast(hashmap_entry_t*, entry->next);
	}

	return false;
}

/**
 * 
 * @function hashmap_remove
 * @brief Remove a key-value pair from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 */
void* hashmap_remove(hashmap_t* map, const char* key)
{
    DEBUG_ME;
	unsigned long hash = hash_function(key);

	size_t index = hash % map->capacity;
	hashmap_entry_t* entry = map->data[index];
	hashmap_entry_t* prev = NULL;

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (prev == NULL) {
				map->data[index] = cast(hashmap_entry_t*, entry->next);
			}
			else {
				prev->next = entry->next;
			}

			void* value = entry->value;

			// TODO: Do we need to destroy the value or not?

			if (entry != NULL) {
				if (entry->key != NULL) {
					memory_destroy(entry->key);
				}

				memory_destroy(entry);
			}

			map->length--;

			return value;
		}

		prev = entry;
		entry = cast(hashmap_entry_t*, entry->next);
	}

	return NULL;
}

/**
 * 
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy(hashmap_t* map)
{
    DEBUG_ME;
	return hashmap_destroy_custom(map, free);
}

/**
 * 
 * @function hashmap_destroy_custom
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @params {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_destroy_custom(hashmap_t* map, void (*free_fn)(void*))
{
    DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t* entry = map->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					memory_destroy(entry->key);

					free_fn(entry->value);

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 * 
 * @function hashmap_print
 * @brief Print the hashmap
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_print(hashmap_t* map)
{
    DEBUG_ME;
	printf("Hashmap Size: %zu\n", map->length);
	// printf("Hashmap Capacity: %zu\n", map->capacity);
	printf("Hashmap Contents:\n");

	for (size_t i = 0; i < map->capacity; i++) {
		hashmap_entry_t* entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: %p\n", i, entry->key, entry->value);
			entry = cast(hashmap_entry_t*, entry->next);
		}
	}
}

/**
 * 
 * @function hashmap_print_custom
 * @brief Print the hashmap with a custom print function
 * @params {hashmap_t*} map
 * @params {void (*print_fn)(void*)} print_fn
 * @returns {void}
 * 
 */
void hashmap_print_custom(hashmap_t* map, void (*print_fn)(void*))
{
    DEBUG_ME;
	printf("Hashmap array: %zu\n", map->length);
	if (map->length == 0) {
		printf("Hashmap is empty\n");
		return;
	}

	for (size_t i = 0; i < map->capacity; i++) {
		hashmap_entry_t* entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: ", i, entry->key);
			print_fn(entry->value);
			printf("\n");

			entry = cast(hashmap_entry_t*, entry->next);
		}
	}
}

/**
 * 
 * @function hashmap_print_layout_attribute
 * @brief Print the hashmap of layout attributes
 * @params {ast_layout_attribute_t*} map - The hashmap to print
 * @returns {void}
 * 
 */
void hashmap_print_layout_attribute(hashmap_layout_attribute_t* map)
{
    DEBUG_ME;
	printf("Hashmap length: %zu\n", map->length);
	if (map->length == 0) {
		printf("Hashmap is empty\n");
		return;
	}

	for (size_t i = 0; i < map->capacity; i++) {
		hashmap_entry_t* entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: ", i, entry->key);
			ast_layout_attribute_t* layout_attribute = entry->value;

			if (layout_attribute != NULL) {
				layout_attribute->print(layout_attribute);
			}
			else {
				printf("NULL\n");
			}

			entry = cast(hashmap_entry_t*, entry->next);
		}
	}
}

/**
 * 
 * @function hashmap_destroy_layout_attribute
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_layout_attribute(hashmap_layout_attribute_t* map)
{
    DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t* entry = map->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					memory_destroy(entry->key);

					ast_layout_attribute_t* layout_attribute = cast(ast_layout_attribute_t*, entry->value);
					if (layout_attribute != NULL) {
						layout_attribute->destroy(layout_attribute);
					}

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 * 
 * @function hashmap_create_layout_attribute
 * @brief Create a new hashmap of layout attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_t*}
 * 
 */
hashmap_layout_attribute_t* hashmap_create_layout_attribute(size_t capacity)
{
	DEBUG_ME;
	hashmap_layout_attribute_t* map = cast(struct hashmap_t*, hashmap_create(capacity));

	map->print = cast(void (*)(void*), hashmap_print_layout_attribute);
	map->destroy = cast(void (*)(void*), hashmap_destroy_layout_attribute);

	return map;
}

/**
 * 
 * @function hashmap_create_layout_attribute_style_state
 * @brief Create a new hashmap of layout style state attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_state_style_t*}
 * 
 */
hashmap_layout_attribute_state_style_t* hashmap_create_layout_attribute_style_state(size_t capacity)
{
	DEBUG_ME;
	hashmap_layout_attribute_state_style_t* map = hashmap_create(capacity);

	map->print = cast(void (*)(void*), hashmap_print_layout_attribute_style_state);
	map->destroy = cast(void (*)(void*), hashmap_destroy_layout_attribute_style_state);

	return map;
}

/**
 * 
 * @function hashmap_print_layout_attribute_style_state
 * @brief Print the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_print_layout_attribute_style_state(hashmap_layout_attribute_t* map)
{
	DEBUG_ME;
	printf("Hashmap style states length: %zu\n", map->length);

	if (map->length == 0) {
		printf("Hashmap style states is empty\n");
		return;
	}
}

/**
 * 
 * @function hashmap_destroy_layout_attribute_style_state
 * @brief Destroy the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_layout_attribute_style_state(hashmap_layout_attribute_t* map)
{
	DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t* entry = map->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					if (entry->key != NULL) {
						memory_destroy(entry->key);
					}

					ast_layout_style_state_t* value = entry->value;
					if (value != NULL) {
						value->destroy(value);
					}

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 * 
 * @function hashmap_layout_attribute_print
 * @brief Print the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_layout_attribute_print(hashmap_layout_attribute_t* map)
{
	DEBUG_ME;
	printf("Hashmap layout attributes length: %zu\n", map->length);

	if (map->length == 0) {
		printf("Hashmap layout attributes is empty\n");
		return;
	}
}

/**
 * 
 * @function hashmap_layout_attribute_destroy
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_layout_attribute_destroy(hashmap_layout_attribute_t* map)
{
	DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t* entry = map->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					if (entry->key != NULL) {
						memory_destroy(entry->key);
					}

					hashmap_layout_attribute_t* value = entry->value;
					if (value != NULL) {
						value->destroy(value);
					}

					memory_destroy(entry);
					
					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 * 
 * @function hashmap_has_any_sub_value_layout_attribute_style_state
 * @brief Check if the hashmap has any sub value layout attribute style state
 * @params {hashmap_layout_attribute_t*} map
 * @returns {bool}
 * 
 */
bool hashmap_has_any_sub_value_layout_attribute_style_state(hashmap_layout_attribute_t* map)
{
	DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t* entry = map->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					ast_layout_style_state_t* value = entry->value;
					if (value != NULL) {
						if (ast_layout_style_state_has_any_sub_value(value)) {
							return true;
						}
					}

					entry = next;
				}
			}
		}
	}

	return false;
}

/**
 * 
 * @function hashmap_has_any_sub_value_layout_attribute
 * @brief Check if the hashmap has any sub value layout attribute
 * @params {hashmap_layout_attribute_t*} map
 * @returns {bool}
 * 
 */
bool hashmap_layout_attribute_has_any_sub_value(hashmap_layout_attribute_t* map)
{
	DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t* entry = map->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					ast_layout_attribute_t* value = entry->value;
					if (value != NULL) {
						if (ast_layout_attribute_has_any_sub_value(value)) {
							return true;
						}
					}

					entry = next;
				}
			}
		}
	}

	return false;
}