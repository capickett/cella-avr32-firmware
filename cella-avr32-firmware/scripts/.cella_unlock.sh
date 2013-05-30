#! /bin/bash

RESFILE="./RES"
WRITEFILE="./CMD"

### begin functions ###

#------------------------------------------------------#
# Waits for the appearance of a response file from the #
# Cella Secure device, and returns its contents        #
#                                                      #
# parameters:                                          #
#   $1 - reponse:   output parameter (contains result) #
#------------------------------------------------------#
readresponse() {
    local __responseresult=$1
    local myresponse=""
    until [ -e "$RESFILE" ]; do
        :
    done
    read resp < "$RESFILE"
    rm "$RESFILE" 
    eval $__responseresult="'$resp'"
}

#------------------------------------------------------------------------#
# Compares two strings and returns whether or not they are equal,        #
# printing optional messages in either case                              #
#                                                                        #
# parameters:                                                            #
#   $1 - string1 :   first string to compare with second                 #
#   $2 - string2 :   second string to compare with first                 #
#   $3 - result  :   output parameter (true if $1 = $2, false otherwise) #
#   $4 - message1:   message to be printed on success (optional)         #
#   $5 - message2:   message to be printed on failure (optional)         #
#------------------------------------------------------------------------#
validate() {
    local __validateresult=$3
    local myresult=false
    if [ "$1" == "$2" ]
      then
        if [ "$4" != '' ]
          then
            echo -e "$4\n"
        fi
        myresult=true
    else
        if [ "$5" != '' ]
          then
            echo -e "$5\n"
        fi
        myresult=false
    fi
    eval $__validateresult="'$myresult'"
}

#------------------------------------------------------------#
# Prints input options and their descriptions to the console #
#------------------------------------------------------------#
usage() {
    echo -e "c\tset configuration"
    echo -e "g\tget configuration"
    echo -e "p\tunlock drive"
    echo -e "n\tchange password"
    echo -e "?\tquery lock state"
    echo -e "l\tlock drive"
}

#--------------------------------------------------------------------#
# Retrieves and prints the configuration of this Cella Secure device #
#--------------------------------------------------------------------#
getconfig() {
    echo "Retrieving configuration..."
    echo "c" > "$WRITEFILE"
    readresponse config
    local index; local option; local value
    for (( index=0; index<${#config}; index++ )); do
        option=${config:$index:1}
        case $option in
        "e") value=${config:$index+1:1}
             index=$(($index + 1))
             echo "Encryption level: $value"
            ;;
        # new configuration options to be added here.
          *) # pass bad characters...
            ;;
        esac
    done
}

### end functions ###

### begin script ###

INPUT="Cellaaaaaa!!!"

while [ "$INPUT" != '' ]; do
    echo -e "Welcome to Cella Secure! Please enter your password. (newline to quit):"
    read -p "> " -s INPUT
    if [ "$INPUT" != '' ]; then
        echo "$INPUT" > "$WRITEFILE"
        echo -e "\nValidating..."
        readresponse resp
        validate "$resp" "K" result "Success!" "Validation failed."
        if $result ; then
            break
        else
            exit
        fi
    fi
done

while [ "$INPUT" != '' ]; do
    echo -e "Enter a command (h for help, newline to quit):"
    read -p "> " INPUT
    case "$INPUT" in
        "h") usage
            ;;
        "c") getconfig
             newconfig='c'
             while true ; do
                 echo "Select option: 1) Encryption level, 2) Done"
                 read -p "> " option
                 case $option in
                     "1") echo "Enter the new encryption level."
                         read -p "> " value
                         if [ $value == "0" ] || [ $value == "1" ] || [ $value == "2" ]; then
                             newconfig="{$newconfig}e$value"
                             echo "Encryption level is now $value"
                         else
                             echo "Invalid encryption level (Must be 0, 1, or 2)"
                         fi
                         ;;
                     # new configuration options to be added here.
                     "2") echo "Updating configuration..."
                         echo "$newconfig" > "$WRITEFILE"
                         readresponse resp
                         validate "$resp" "K" result "Update complete." "Update failed."
                         break
                         ;;
                     *) ;;
                 esac
             done
            ;;
        "g") getconfig
            ;;
        "p") echo -e "Attempting to unlock, please enter password:"
             read -p "> " -s PASS
             echo "p$PASS" > "$WRITEFILE"
             echo -e "Validating..."
             readresponse resp
             validate "$resp" "K" result "Your drive is now unlocked!" "Validation failed."
            ;;
        "n") echo "Attemping to change password, please enter your OLD password."
             read -p "> " -s OLDPASS; echo
             NEWPASS1="foo"
             NEWPASS2="bar"
             while [ "$NEWPASS" != "$OLDPASS" ] && [ "$OLDPASS" != '' ]; do
                 echo "Please enter your NEW password (newline to abort)."
                 read -p "> " -s NEWPASS1; echo
                 if [ "$NEWPASS1" != '' ]; then
                     echo "Please enter your NEW password again."
                     read -p "> " -s NEWPASS2; echo
                 fi
                 if [ "$NEWPASS1" == '' ] || [ "$NEWPASS2" == '' ]; then
                     echo -e "Aborting.\n"
                     break
                 elif [ "$NEWPASS1" != "$NEWPASS2" ]; then
                     echo -e "Your passwords did not match!\n"
                 else
                     echo "n$OLDPASS$NEWPASS1" > "$WRITEFILE"
                     echo "Validating..."
                     readresponse resp
                     validate "$resp" "K" result "Your password has been updated." "Validation failed!"
                     break
                 fi
             done
             echo
            ;;
        # hold up. This seems silly.
        "?") echo "?" > "$WRITEFILE"
             echo -e "Querying lock status..."
             readresponse resp
             validate "$resp" "?U" result "This drive is unlocked"
             if [ ! $result ]; then
                 validate "$resp" "?L" result "This drive is locked!" "Whoops, we got a bad response"
             fi
            ;;
        "l") echo "l" > "$WRITEFILE"
             echo -e "Locking drive..."
             readresponse resp
             validate "$resp" "K" result "Your drive is now locked." "Locking failed."
            ;;
         "") # echo "l" > "$WRITEFILE" # uncomment to lock drive on close
             read -p "Are you sure you want to quit? (y/n) " conf
             if [ "$conf" == "y" ] || [ "$conf" == "yes" ]; then
               echo "Goodbye!"
             else
                INPUT="cella!"
             fi
            ;;
        *) echo "Unrecognized command: $c"
    esac
done
