#ifndef DDI_XML_H
#define DDI_XML_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct xml_elem;
typedef struct xml_elem xml_elem;

typedef struct xml_document xml_document;


const char *xml_get_text(xml_elem *node);

const char *xml_get_tag(struct xml_elem* node);

xml_elem *xml_first(xml_elem *elem, const char *tag);

xml_elem *xml_next(xml_elem *elem, const char *name);

xml_elem *xml_find_elem(xml_elem *elem, const char *path);

size_t xml_get_string(xml_elem *e, const char *str, char *dst);

const char *xml_get_attrib(xml_elem *node, const char *attrib);

char *xml_parse_attrib(xml_elem *node, char *p);

char *xml_parse_node(xml_elem *node, char *p);

xml_document *xml_parse_document(const char *data, size_t len, int copy);

void xml_free_document(xml_document *doc);

xml_elem *xml_root(xml_document *xml);

size_t xml_attribute_count(xml_elem *elem);

#ifdef __cplusplus
}
#endif

#endif // DDI_XML_H
