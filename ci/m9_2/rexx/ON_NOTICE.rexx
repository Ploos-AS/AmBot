/* M9.2 test hook: returns non-zero to verify hook isolation. */
PARSE ARG network source target text
SAY "M9.2 FAILHOOK network=" network " source=" source " target=" target
EXIT 10
