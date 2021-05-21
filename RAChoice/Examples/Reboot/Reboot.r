; $VER: Reboot.r 1.0 (20.5.2021)
; This script pops a Reboot message requester

; Use a predefined icon
;C:RAChoice TITLE="Question" BODY="Reboot the computer now ?" GADGETS="_Reboot now|_Cancel" IMAGE="WARNING" SET="RESULT"

; Use an icon file
C:RAChoice TITLE="Question" BODY="Reboot the computer now ?" GADGETS="_Reboot now|_Cancel" IMAGE="Reboot.r.info" SET="RESULT"

; Use a picture file
;C:RAChoice TITLE="Question" BODY="" GADGETS="_Reboot now|_Cancel" IMAGE="Reboot.r.PNG" SET="RESULT"

IF ${RESULT} GE 1
    C:Reboot
ENDIF

QUIT
