// This XML parser aspires to, someday, follow the syntax rules from https://www.w3.org/TR/REC-xml/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ddi_xml.h"
#include "ddi_str_utils.h"

#define LOG_ERROR     1
#define LOG_DEBUG     2

static int log_level = LOG_ERROR;

#define FATAL(...) do { printf("[XML] ERROR: " __VA_ARGS__); exit(1); }while(0)
#define ELOG(...) do { if (log_level >= LOG_ERROR) printf("[XML] ERROR: " __VA_ARGS__); }while(0)
#define DLOG(...) do { if (log_level >= LOG_DEBUG) printf("[XML] " __VA_ARGS__); }while(0)

typedef struct
{
  const char *name;
  const char *value;
} string_pair;

struct xml_elem
{
  const char *tag;
  const char *text;
  size_t attr_count;
  string_pair *attributes;
  size_t child_count;
  xml_elem* children;
};
struct xml_document
{
  char *buffer;
  xml_elem root;
};

static void *zalloc(size_t size)
{
  void *p = malloc(size);
  if (!p)
  {
    FATAL("malloc(%d) failed\n", (int)size);
  }
  memset(p, 0, size);
  return p;
}

const char *xml_get_text(xml_elem *node)
{
  return node ? node->text : 0;
}

const char *xml_get_tag(struct xml_elem* node)
{
  return node->tag;
}

xml_elem *xml_root(xml_document *xml)
{
  return xml ? &xml->root : 0;
}

size_t xml_attribute_count(xml_elem *elem)
{
  return elem ? elem->attr_count : 0;
}

xml_elem *xml_first(xml_elem *elem, const char *tag)
{
  if (elem)
  {
    if (!tag) // no tag means get the first child
      return elem->children;

    // search for the first child with a matching tag
    if (elem->child_count && elem->children)
    {
      xml_elem *child = elem->children;
      xml_elem *last = child + elem->child_count;
      while (child < last)
      {
        if (0 == strcmp(tag, child->tag))
          return child;
        child++;
      }
    }
  }
  return 0;
}

xml_elem *xml_next(xml_elem *elem, const char *tag)
{
  if (elem)
  {
    elem++;
    if (!tag)
      return elem->tag ? elem : 0;

    while (elem->tag)
    {
      if (0 == strcmp(tag, elem->tag))
        return elem;
      elem++;
    }
  }
  return 0;
}

xml_elem *xml_find_elem(xml_elem *elem, const char *path)
{
  char tag[64]; // temporary buffer
  const char *p = path;
  char *s;
  int i = 63;

  do {
    s = tag;
    // copy a path segment
    while ((*p != '/') && *p && i--)
      *s++ = *p++;
    *s = '\0';
    elem = xml_first(elem, tag);
  } while (elem && *p++);

  return elem;
}

size_t xml_get_string(xml_elem *e, const char *str, char *dst)
{
  size_t len = 0;
  if (!e || !(e = xml_first(e, str)))
    return 0;
  const char *name = xml_get_text(e);
  if (name)
  {
    len = strlen(name);
    if (dst) {
      memcpy(dst, name, len + 1);
    }
  }
  return len;
}

const char *xml_get_attrib(xml_elem *node, const char *attrib)
{
  if (node && node->attributes)
  {
    size_t i = node->attr_count;
    string_pair *p = node->attributes;
    while (i > 0)
    {
      if (p->name && (0 == strcmp(attrib, p->name)))
        return p->value;
      p++;
      i--;
    }
  }
  return 0;
}

char *xml_parse_attrib(xml_elem *node, char *p)
{
  node->attr_count++;
  size_t size = node->attr_count * sizeof(string_pair);
  string_pair *sp = (string_pair *)malloc(size);
  if (node->attributes)
  {
    memcpy(sp, node->attributes, size - sizeof(string_pair));
    free(node->attributes);
  }
  node->attributes = sp;
  sp = &sp[node->attr_count - 1];

  sp->name = p;
  while (*p != '=')
    p++;
  *p++ = 0; // terminate attribute string (over write '=' char)

  char c = *p++; // get opening single or double quote char
  sp->value = p;
  while (*p != c) // scan for closing quote
    p++;
  *p++ = 0; // terminate value string (over write quote char)
  DLOG("attribute: %s = %s\n", sp->name, sp->value);
  p = skip_spaces(p);
  return p;
}

void xml_ensure_capacity(xml_elem *node, size_t count)
{
  if (count <= node->child_count)
    return;

  count++; // add one for termination elem
  size_t size = (count * sizeof(xml_elem));
  xml_elem *child = node->children;
  node->children = (xml_elem *)malloc(size);

  if (node->child_count)
  {
    if (!child)
    {
      ELOG("memory hosed\n");
    }

    size = (node->child_count * sizeof(xml_elem));
    memcpy(node->children, child, size);
    free(child);
  }

  child = &node->children[node->child_count];
  size = (count - node->child_count) * sizeof(xml_elem);
  memset(child, 0, size);
}

char *xml_parse_tag(xml_elem *node, char *p)
{
  char c = *p; // p should always point to the first char of the tag string: one char past the <

  node->tag = p;
  while (c && (c != ' ') && (c != '>') && (c != '/')) c = *(++p);
  *p = 0; // terminate tag string
  DLOG("NODE: \"%s\"\n", node->tag);

  while (c != '>') // space before attribute or closed node
  {
    if (c == ' ') // skip past space after tag: <tag /> or <tag attrib="value"
    {
      p = skip_spaces(++p);
      c = *p;
    }

    if (c == '/') // end closed node "/>"
    {
      DLOG("end closed node: \"%s\"\n", node->tag);
      break;
    }

    if (isalpha(c) || (c == ':') || (c == '_'))
    {
      p = xml_parse_attrib(node, p);
      c = *p;
    }
  }
  return p;
}

char *skip_comment(char *p)
{
  char *start = p;
  if ((p[1] != '-') || (p[2] != '-'))
  {
    p = scan_to(p, '>');
    *(++p) = 0;
    FATAL("malformed comment <%s\n", start);
  }
  p += 3;
  do {
    p = scan_to(p, '-');
    if (p[1] == '-')
    {
      if (p[2] == '>')
      {
        p += 3;
        break;
      }

      p = scan_to(p, '>');
      *(++p) = 0;
      FATAL("malformed comment <%s\n", start);
    }
  } while (p++);

  p = skip_spaces(p);
  if (*p != '<')
  {
    p = scan_to(p, '>');
    *(++p) = 0;
    FATAL("malformed comment <%s\n", start);
  }
  return p;
}

char *xml_parse_node(xml_elem *node, char *p)
{
  char c; // p should always point to the first char of the tag string: one char past the <

  p = xml_parse_tag(node, p);
  c = *p;
  if (c == '/') // end closed node "/>"
  {
    return p + 1;
  }

  // scan element content text
  p = skip_spaces(++p);
  node->text = p;
  p = scan_to(p, '<');
  *p++ = 0;
  DLOG("NODE: text: \"%s\"\n", node->text);

  // p should always point to one char past the <
  do {
    if (*p == '/') // close end tag? </tag>
    {
      char *e = ++p;
      e = scan_to(e, '>'); // scan to end of closing tag
      int len = (int)(e - p);
      if (0 == strncmp(node->tag, p, len))
      {
        DLOG("NODE: \"%s\" end\n", node->tag);
        return e;
      }
      *e = 0;
      ELOG("unbalanced closing tag: \"%s\" terminating scan\n", p);
      return p;
    }

    if (*p == '!') // skip any <!-- comment -->
    {
      p = skip_comment(p);
      c = *(p++);
      continue;
    }

    xml_ensure_capacity(node, node->child_count + 1);
    xml_elem *child = &node->children[node->child_count];
    node->child_count++;

    p = xml_parse_node(child, p);
    c = *p++;
    if (c == '>')
    {
      p = skip_spaces(p);
      c = *p++;
    }
  } while (c == '<');

  return p;
}

xml_document *xml_parse_document(const char *data, size_t len, int copy)
{
  xml_document *xml;
  char *p, *end;
  xml = (xml_document *)zalloc(sizeof(xml_document));
  if (!xml)
    return 0;

  if (copy)
  {
    xml->buffer = (char *)malloc(len);
    if (!xml->buffer)
      return 0;
    memcpy(xml->buffer, data, len);
    data = xml->buffer;
  }

  p = (char *)data;
  end = (char *)(data + len);

  while ((p < end) && *p)
  {
    if (*p++ == '<')
    {
      if ((*p != '?') && (*p != '!'))
        break;

      while ((p < end) && (*p++ != '>')) { ; } // scan to end of prolog or comment
    }
  }

  p = xml_parse_node(&xml->root, p);
  if (p >= end)
  {
    ELOG("parse root end:%p < %p len:%d\n", p, end, (int)len);
  }

  return xml;
  }

void xml_free_node(xml_elem *node)
{
  if (!node)
  {
    ELOG("xml_free_node null\n");
    return;
  }

  if (node->attributes)
  {
    free(node->attributes);
    node->attributes = 0;
  }

  xml_elem *child = node->children;
  if (child)
  {
    while(child->tag)
    {
      xml_free_node(child);
      child++;
    }
    free(node->children);
    node->children = 0;
  }
    }

void xml_free_document(xml_document *doc)
{
  if (doc)
  {
    xml_free_node(&doc->root);
    if (doc->buffer)
      free(doc->buffer);
    doc->buffer = 0;
    free(doc);
  }
}
