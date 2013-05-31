#! /bin/bash

#------------------------------------------#
# CLI for Cella clients running Linux/Unix #
#------------------------------------------#

### begin constants ###

RESFILE="./__cellaRES"
CMDFILE="./__cellaCMD"
LOCKFILE="./__cellaLOCK"
MAX_PASS_LENGTH=32
MAX_FAILED_ATTEMPTS=3

### end constants ###

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

#-------------------------------------------------#
# Echo input to outstream with length padded to   #
# at least length.  If the input length is        #
# greater than length, this simply prints input   #
#                                                 #
# parameters:                                     #
#   $1 - input:   unpadded input                  #
#   $2 - length:  minimum length of padded input  #
#   $3 - newline: if true, end input with newline #
#-------------------------------------------------#
padinput() {
    local index
    echo -n "$1"
    for (( index=${#1} ; index <= $2 ; index++ )) ; do
        echo -ne "\0"
    done
    if $3; then
        echo
    fi
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
    if [ "$1" == "$2" ]; then
        if [ -n "$4" ]; then
            echo -e "$4\n"
        fi
        myresult=true
    else
        if [ -n "$5" ]; then
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
    echo -e "n\tchange password"
}

#--------------------------------------------------------------------#
# Retrieves and prints the configuration of this Cella Secure device #
#--------------------------------------------------------------------#
getconfig() {
    echo "Retrieving configuration..."
    touch "$LOCKFILE"
    echo "c" > "$CMDFILE"
    rm "$LOCKFILE"
    readresponse config
    local index; local option; local value
    for (( index=0; index<${#config}; index++ )); do
        option=${config:$index:1}
        case $option in
        "e") value=${config:$index+1:1}
             index=$(($index + 1))
             echo "Encryption level: $value"
            ;;
          *) # pass bad characters...
            ;;
        esac
    done
    echo
}

#--------------------------------------------------------#
# Lock drive, cleanup temporary files, and exit properly #
#--------------------------------------------------------#
graceful_exit() {
    rm -f "$RESFILE"

    # if we trap a signal in silent mode, the terminal gets stuck
    # in silent mode. This hack ensures that doesn't happen
    echo "uglyhack" | read
    exit 0
}

### end functions ###

### begin script ###

input="Cellaaaaaa!!!"

trap graceful_exit SIGINT SIGTERM SIGHUP SIGSTOP

failed=0
while [ -n "$input" ]; do
    echo -e "Welcome to Cella Secure! Please enter your password. (newline to quit):"
    read -p "> " -s input
    if [ -n "$input" ]; then
        if [ ${#input} -le $MAX_PASS_LENGTH ]; then
            touch "$LOCKFILE"
            padinput "p$input" $MAX_PASS_LENGTH+1 true > "$CMDFILE"
            rm "$LOCKFILE"
            echo -e "\nValidating..."
            readresponse resp
            validate "$resp" "K" result "Success!" "Validation failed."
            if $result ; then
                break
            elif [ $failed -ge $MAX_FAILED_ATTEMPTS ]; then
                echo "Received too many incorrect passwords! Goodbye!"
                exit 0
            else
                failed=$(($failed + 1))
            fi
        else
            echo "Validation failed, please try again."
            failed=$(($failed + 1))
        fi
    fi
done

while [ -n "$input" ]; do
    echo -e "Enter a command (h for help, newline to quit):"
    read -p "> " input
    case "$input" in
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
                              newconfig="e$value"
                              echo "Encryption level will be set to $value"
                          else
                              echo "Invalid encryption level (Must be 0, 1, or 2)"
                          fi
                         ;;
                     # new configuration options to be added here.
                     "2") echo "Updating configuration..."
                          touch "$LOCKFILE"
                          echo "$newconfig" > "$CMDFILE"
                          rm "$LOCKFILE"
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
        "n") echo "Attemping to change password, please enter your OLD password."
             read -p "> " -s OLDPASS; echo
             NEWPASS1="foo"
             NEWPASS2="bar"
             while [ "$NEWPASS1" != "$NEWPASS2" ] && [ -n "$OLDPASS" ]; do
                 echo "Please enter your NEW password (newline to abort)."
                 read -p "> " -s NEWPASS1; echo
                 if [ -n "$NEWPASS1" ]; then
                     echo "Please enter your NEW password again."
                     read -p "> " -s NEWPASS2; echo
                 fi

                 if [ -z "$NEWPASS1" ] || [ -z "$NEWPASS2" ]; then
                     echo -e "Aborting.\n"
                     break
                 elif [ ${#NEWPASS1} -gt $MAX_PASS_LENGTH ] || [ ${#OLDPASS} -gt $MAX_PASSLENGTH ]; then
                     echo -e "Your password is too long! Please keep it below 32 characters.\n"
                 elif [ "$NEWPASS1" != "$NEWPASS2" ]; then
                     echo -e "Your passwords did not match!\n"
                 else
                     touch "$LOCKFILE"
                     padinput "n$OLDPASS" MAX_PASS_LENGTH+1 false > "$CMDFILE"
                     padinput "$NEWPASS" MAX_PASS_LENGTH+1 true > "$CMDFILE"
                     rm "$LOCKFILE"
                     echo "Validating..."
                     readresponse resp
                     validate "$resp" "K" result "Your password has been updated." "Validation failed!"
                     break
                 fi
             done
             echo
            ;;
         "") read -p "Are you sure you want to quit? (y/n) " conf
             if [ "$conf" == "y" ] || [ "$conf" == "yes" ]; then
               echo -e "Goodbye!\n"
             else
                INPUT="cella!"
             fi
            ;;
        *) echo "Unrecognized command: $c"
    esac
done
exit 0
