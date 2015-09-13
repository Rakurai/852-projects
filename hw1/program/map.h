#ifndef MAP_H
#define MAP_H

typedef unsigned int map_key_t;
typedef void * map_value_t;

typedef struct map_element_st {
    map_key_t key;
    map_value_t value;

    struct map_element_st *next;
} map_element_t;

typedef struct map_st {
    map_element_t *head;
    int size;
} map_t;

void map_init(map_t *m) {
    m->head = NULL;
    m->size = 0;
}

void map_clear(map_t *m) {
    while (m->head) {
        map_element_t *el = m->head;
        m->head = el->next;
        free(el);
    }

    m->size = 0;
}

void map_put(map_t *m, map_key_t key, map_value_t value) {
    map_element_t *el = malloc(sizeof(map_element_t));
    el->key = key;
    el->value = value;

    el->next = m->head;
    m->head = el;
    m->size++;
}

map_value_t map_remove(map_t *m, map_key_t key) {
    map_element_t *el, *prev = NULL;
    map_value_t value = NULL;

    for (el = m->head; el; prev = el, el = el->next)
        if (el->key == key)
            break;

    if (el) {
        if (prev)
            prev->next = el->next;
        else
            m->head = el->next;

        value = el->value;
        free(el);
        m->size--;
    }

    return value;
}

map_value_t map_get(map_t *m, map_key_t key) {
    map_element_t *el;

    for (el = m->head; el; el = el->next)
        if (el->key == key)
            return el->value;

    return NULL;
}

map_value_t map_pop(map_t *m) {
	map_element_t *el = m->head;
	if (el)
		m->head = el->next;
	return el->value;
}

#endif // MAP_H
