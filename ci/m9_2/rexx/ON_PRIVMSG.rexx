/* M9.2 ON_PRIVMSG evidence hook. Arguments are supplied by AmBot. */
PARSE ARG network source target text
SAY "M9.2 HOOK network=" network " source=" source " target=" target " text=" text
EXIT 0
