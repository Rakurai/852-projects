#ifndef VECTOR_H
#define VECTOR_H

typedef void * vector_element_t;

typedef struct vector_st {
    vector_element_t *data;
    unsigned int size;
} vector_t;

void vector_resize(vector_t *v, unsigned int size) {
	vector_element_t *new_data = NULL;

	if (size > 0) {
		new_data = calloc(size, sizeof(vector_element_t));

		if (v->data) {
			unsigned int copy_bytes = v->size < size ? v->size : size;
			memcpy(new_data, v->data, copy_bytes * sizeof(vector_element_t));
		}
	}

	if (v->data)
		free(v->data);

	v->data = new_data;
	v->size = size;
}

void vector_clear(vector_t *v) {
	vector_resize(v, 0);
}

void vector_init(vector_t *v, unsigned int size) {
	v->data = NULL;
	v->size = 0;
	vector_resize(v, size);
}

void vector_set(vector_t *v, unsigned int index, vector_element_t value) {
	if (index >= v->size)
		vector_resize(v, (v->size <= 0 ? 1 : v->size) * 2);

	v->data[index] = value;
}

vector_element_t *vector_get(vector_t *v, unsigned int index) {
	if (index >= v->size)
		return NULL;

	return v->data[index];
}

#endif // VECTOR_H
