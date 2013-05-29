#! /bin/bash

WRITEFILE="./CMD"
RESFILE="./RES"

function readresponse(){
    until [ -f "$RESFILE" ]; do
        :
    done
    read resp < "$RESFILE"
#   Comment this line for debugging
    rm "$RESFILE" 
    echo "$resp"
}

function help(){
    echo -e "c\tset configuration"
    echo -e "g\tget configuration"
    echo -e "p\tunlock drive"
    echo -e "n\tchange password"
    echo -e "?\tquery lock state"
    echo -e "l\tlock drive"
}

function getconfig(){
    echo "Retrieving configuration..."
    echo "c" > "$WRITEFILE"
    CONFIG=$(readresponse)
# TODO: parse configuration
}

function setconfig() {
    getconfig
# TODO: display options
}

INPUT="Cellaaaaaa!!!"

while [ "$INPUT" != '' ]
do
    echo -e "Welcome to Cella Secure! Please enter your password. (newline to quit):"
    read -p "> " -s INPUT
    echo
    if [ "$INPUT" != '' ]
      then
        echo "$INPUT" > "$WRITEFILE"
        echo "Validating..."
        RESP=$(readresponse)
        if [ "$RESP" = "K" ]
          then
            echo -e "Success!\n"
            break
        else
            echo -e "Validation failed.\n"
        fi
    fi
done

while [ "$INPUT" != '' ]
do
    echo -e "Enter a command (h for help, newline to quit):"
    read -p "> " INPUT
    case "$INPUT" in
        "h") help
            ;;
        "c") setconfig
            ;;
        "g") getconfig
            ;;
        "p") echo -e "Attempting to unlock, please enter password:"
             read -p "> " -s PASS
             echo "p$PASS" > "$WRITEFILE"
             echo -e "Validating..."
             RESP=$(readresponse)
             if [ "$RESP" = "K" ]
               then
                 echo -e "Success! Your drive is now unlocked.\n"
             else
                 echo -e "Validation failed.\n"
             fi
            ;;
        "n") echo "Attemping to change password, please enter your OLD password"
             read -p "> " -s OLDPASS
             NEWPASS1="foo"
             NEWPASS2="bar"
             while [ "$NEWPASS" != "$OLDPASS" ] && [ "$OLDPASS" != '' ]
               do
                 echo "Please enter your NEW password (newline to abort)"
                 read -p "> " -s NEWPASS1
                 if [ "$NEWPASS1" != '' ]
                   then
                     echo "Please enter your NEW password again"
                     read -p "> " -s NEWPASS2
                 fi
                 if [ "$NEWPASS1" == '' ] || [ "$NEWPASS2" == '' ]
                   then
                     echo -e "Aborting.\n"
                     break
                 elif [ "$NEWPASS1" != "$NEWPASS2" ]
                   then
                     echo -e "Your passwords did not match!\n"
                 else
                     echo "n$OLDPASS$NEWPASS1" > "$WRITEFILE"
                     echo "Validating..."
                     RESP=$(readresponse)
                     if [ "$RESP" = "K" ]
                       then
                         echo -e "Success! Your password has been updated.\n"
                     else
                         echo -e "Validation failed.\n"
                     fi
                     break
                 fi
             done
             echo
            ;;
        # hold up. This seems silly.
        "?") echo "?" > "$WRITEFILE"
             echo -e "Querying lock status..."
             RESP=$(readresponse)
             if [ "$RESP" = "?U" ]
               then
                 echo -e "This drive is unlocked!\n"
             elif [ "$RESP" = "?L" ]
               then
                 echo -e "This drive is locked!\n"
             else
                 echo -e "Whoops, we got a bad response.\n"
             fi
            ;;
        "l") echo "l" > "$WRITEFILE"
             echo -e "Locking drive..."
             RESP=$(readresponse)
             if [ "$RESP" = "K" ]
               then
                 echo -e "Your drive is now locked.\n"
               else
                 echo -e "Locking failed.\n"
             fi
            ;;
         "") # echo "l" > "$WRITEFILE" # uncomment to lock drive on close
             echo "Goodbye!"
            ;;
        *) echo "Unrecognized command: $c"
    esac

done
