;----------------------------------------------------------
; Display VControl output in a V47+ Reaction Requester
; $VER: VControl.rt 1.0 (20.5.2021) Philippe CARPENTIER
;----------------------------------------------------------

SET BODY  "T:BODY"
SET TITLE "VControl"

;----------------------------------------------------------
; VCONTROL > MAIN LOOP
;----------------------------------------------------------

LAB VCONTROL_LOOP

Echo ""                                        >  ${BODY}
Echo ""                                        >> ${BODY}
C:Version C:VControl FILE FULL                 >> ${BODY}
Echo ""                                        >> ${BODY}
Echo "Select some VControl information below." >> ${BODY}
Echo ""                                        >> ${BODY}

SET ICON    "VControl.r.info"
SET GADGETS "_Board|_Config|_CPU|_Memory|M_odules|_IDE|_Help|_Cancel"

C:RAChoice TITLE="${TITLE}" BODY="${BODY}" GADGETS="${GADGETS}" IMAGE="${ICON}" SET="RESULT" FILE

IF ${RESULT} GE 1
    
    SET ICON    "INFO"
    SET GADGETS "_Thanks"
    
    IF ${RESULT} EQ 1
        C:VControl BO > ${BODY}
        SET ICON "VControl_BOARD.info"
    ENDIF
    
    IF ${RESULT} EQ 2
        C:VControl CD > ${BODY}
        SET ICON "VControl_BOARD.INFO"
    ENDIF
    
    IF ${RESULT} EQ 3
        C:VControl CP > ${BODY}
        SET ICON "VControl_CPU.info"
    ENDIF
    
    IF ${RESULT} EQ 4
        C:VControl ML > ${BODY}
        SET ICON "VControl_MEMLIST.info"
    ENDIF
    
    IF ${RESULT} EQ 5
        C:VControl MO > ${BODY}
        SET ICON "VControl_MODULES.info"
    ENDIF
    
    IF ${RESULT} EQ 6
        SKIP VCONTROL_IDESPEED
    ENDIF
    
    IF ${RESULT} EQ 7
        C:VControl > ${BODY}
        SET ICON VControl_HELP.info
    ENDIF
    
    C:RAChoice TITLE="${TITLE}" BODY="${BODY}" GADGETS="${GADGETS}" IMAGE="${ICON}" FILE >NIL:
    
    SKIP VCONTROL_LOOP BACK
    
ENDIF

C:Delete QUIET "${BODY}" >NIL:

QUIT

;----------------------------------------------------------
; VCONTROL > IDE SPEED
;----------------------------------------------------------

LAB VCONTROL_IDESPEED

ECHO "Set the Vampire SCSI-IDE speed mode.*n" >  "${BODY}"
ECHO "Use this feature with caution!*n"       >> "${BODY}"
ECHO "0: Slow    (~ 3MB/sec)"                 >> "${BODY}"
ECHO "1: Fast    (~ 6MB/sec)"                 >> "${BODY}"
ECHO "2: Faster  (~ 8MB/sec)"                 >> "${BODY}"
ECHO "3: Fastest (~10MB/sec)"                 >> "${BODY}"

SET ICON    "VControl_IDESPEED.info"
SET GADGETS "Speed _0|Speed _1|Speed _2|Speed _3|_Cancel"

C:RAChoice TITLE="${TITLE}" BODY="${BODY}" GADGETS="${GADGETS}" IMAGE="${ICON}" FILE SET="RESULT"

IF ${RESULT} GE 1

    SET ICON    "INFO"
    SET GADGETS "_Thanks"
    
    IF ${RESULT} EQ 1
        C:VControl ID 0 > ${BODY}
    ENDIF
    
    IF ${RESULT} EQ 2
        C:VControl ID 1 > ${BODY}
    ENDIF
    
    IF ${RESULT} EQ 3
        C:VControl ID 2 > ${BODY}
    ENDIF
    
    IF ${RESULT} EQ 4
        C:VControl ID 3 > ${BODY}
    ENDIF
    
    C:RAChoice TITLE="${TITLE}" BODY="${BODY}" GADGETS="${GADGETS}" IMAGE="${ICON}" FILE >NIL:
    
ENDIF

SKIP VCONTROL_LOOP BACK

;----------------------------------------------------------
; END OF SCRIPT
;----------------------------------------------------------
