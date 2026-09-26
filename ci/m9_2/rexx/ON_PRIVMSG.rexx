/* M9.2 ON_PRIVMSG evidence hook. Arguments are supplied by AmBot. */
PARSE ARG network source target text
SAY "M9.2 HOOK network=" network " source=" source " target=" target " text=" text
ADDRESS COMMAND 'Echo "M9.2 HOOK" >>AMBOTQ:evidence/hooks.log'
IF text = "M9_2_AFTER_FAIL" THEN ADDRESS COMMAND 'Echo "M9_2_AFTER_FAIL" >>AMBOTQ:evidence/hooks.log'
EXIT 0
