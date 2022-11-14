
#COLOR_START
FGREP  = $(shell which fgrep)
RED    = "\033[0;91m"
GREEN  = "\033[1;92m"
YELLOW = "\033[1;93m"
ORANGE = "\033[0;93m"
BLUE   = "\033[1;94m"
VIOLET = "\033[0;95m"
CYAN   = "\033[0;96m"
CLEAR  = "\033[0m"

SECT_COLOR = $(YELLOW)
CMD_COLOR = $(VIOLET)
DESC_COLOR = $(GREEN)

COLORTAGS = awk '{ \
  gsub("\\[!\\]",$(RED)"&"$(CLEAR)); \
  gsub("\\[yellow\\]",$(YELLOW)); \
  gsub("\\[red\\]",$(RED)); \
  gsub("\\[blue\\]",$(BLUE)); \
  gsub("\\[violet\\]",$(VIOLET)); \
  gsub("\\[cyan\\]",$(CYAN)); \
  gsub("\\[clear\\]",$(CLEAR)); \
  print}'

COLORHELP = awk '{matched=0} /^[^ ].*:.*/ \
  {print($(CMD_COLOR)substr($$1,0,length($$1)-1)$(CLEAR)":"$(DESC_COLOR)substr($$0, length($$1)+1,length($$0))$(CLEAR));matched=1} \
  {if (!matched) print($(DESC_COLOR)$$0$(CLEAR))}'

define HELP
$(FGREP) -h "##" $(1) | $(FGREP) -v fgrep | sed -e 's/\\$$//' | sed -e 's/##//'|$(COLORTAGS)|$(COLORHELP)
endef
#COLOR_END

