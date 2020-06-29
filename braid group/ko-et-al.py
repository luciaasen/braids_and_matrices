from sage.all import *
import re
import sys

###########################################################
#         FUNCION PARA PASAR DE TRENZAS A LISTAS
#        'a0*a1*a4^-3*a2^2' -> [1,2,-5,-5,-5,3,3]
###########################################################

# Una punietera funcion que dada una trenza de la forma a1^2*a2^-20*a15^21, 
# returns the list associated to that braid (in this case, list would be [0,0,-1,-1,-1...]
# so that string '=' b(list)) where b is the braid group of n strands 
# (si estas en B3 y me pasas un a15 NOT MY FAULT)
# letter indicates the letter with which you instantiated the braid group (i.e., b ) BraidGroup(n, letter)
# Me cago en la leche wue ya podrian darme esto hecho, AQUI YO COMO UNA GILIPOLLAS QUITANDOME TIEMPO DE 
# PRACTICAS PARA PARSEAR TRENZAS. CACA.
def str_to_list(string, letter = 'a'):
    string = string.replace(letter, '')
    # a0/a1 == braid([a0, -a1])
    string = string.replace('/', '*-')
    # a0^-1 == braid([-a0])
    # places contains thr position of each ^, and 
    places = [m.start() for m in re.finditer('\^[-]?[0-9]+', string)]
    # now, powers contains the number associated to each ^ pointed by places
    powers = [the_power(place, string) for place in places]
    # repes is a list of the strings that shall replace the '^n' inside the input string
    # for example, if '2^3' in string, there will be an element in repes of the form '*2*2*2'
    # similarly, if '3^-3' in string, there will be an element in repes of the form '*-3*-3*-3'
    repes = [repe(string, place, power) for (place, power) in zip(places, powers)]
    # now, we should 'erase' every gen^n from the string and replace it with the correct
    # element from repes (*gen*gen*gen...). First we separate the initial string
    splitted = re.split('[0-9]+\^[-]?[0-9]+', string)
    # Then we introduce the 'repes' between the separations
    without_powers = ''
    for (splt, rep) in zip(splitted, repes):
        without_powers = without_powers + splt + rep
    without_powers += splitted[-1]
    # YYYYY PORFIN, the only thing left to do is to take away the * and to sum one up to each number
    # Por alguna extrania razon que no alcanzo a comprender, a0*a3 = [1,4] --> sin sentidos de la vida de python
    braid_list = [plus_one(elem) for elem in without_powers.split('*')]
    return braid_list

#############################################################
##### FUNCIONES AUXILIARES PA LA JAMADA DE AHI ARRIBA #######
#############################################################

# Given a string and if ini points to a '^', the power returns the number that comes after '^'
def the_power(ini,string):
    #end contains the index of the first * after the ^. That is, the end of the number after the ^
    end = string.find('*', ini)
    if end != -1:
        return int(string[ini+1:end])
    # if end = -1, no '*' was found after '^' --> the number is at the end of the string
    return int(string[ini+1:])

# string is of the form 0^13*1^-2*3...(coming from a0^13*a1^-2...)
# ini points to the ^ and power is an integer describing the number
# just after ^
def repe(string, ini, power):
    if power > 0:
        operator = '*'
    else:
        operator = '*-'
    # The generator is the number just before the '^', (a0^3)
    gen = string[ini-1]
    
    # ret will be '*gen*gen*gen...' as many times as power indicates
    ret = ''
    for i in range(abs(power)):
        ret = ret + operator + gen
    # We take away the initial '*' porq si no se lia :D (Que feo :/)
    return ret[1:]

# elem is a string, either 'i' or '-i', i > 0
# returns integer i+1 in the first case, -i-1 in the second one
def plus_one(elem):
    # special case of elem = '-0' should return -1, not 1
    if str(elem) == '-0':
        return -1
    elif int(elem) >= 0: 
        
        return int(elem) + 1
    return int(elem) - 1


# returns b1 * b2 * b1^-1, where b1, b2 are braid objects
def conjugate(b1, b2):
    return b1*b2/b1

#############################################################
######    IMPLEMENTACION ALGORITMO KEY EXCHANGE       #######
#############################################################

def main():
    n = 10
    b = BraidGroup(n, 'a')

    print('The generators are ' + str(b.gens()))
     
    
    if sys.argv[1] == 'alice':

        # Generate public key
        #public = [randint(-(n-1), n-1) for i in range(7)]
        #while 0 in public:
        #    public.remove(0)
        public = [2,4,-5,-5,1,9,9,9,3]
        print('\nI\'m Alice and the random public key is ' + str(b(public)) + '\n')
        
        print('Also, I will randomly generate my private key using ' + str(b.gens()[n/2:]))
        print('\nGenerating private key... ')
        
        # Private key for Alice in the upper subgroup (< a_n/2+1 ... a_n-1>)
        list_to_choose = range(-(n-1), -(n/2)) + range(n/2+1, n)
        #a_priv = [list_to_choose[randint(0, len(list_to_choose)-1)] for i in range(7)]
        a_priv = [-9,-9,7,6,6,6,-8]
        print('Done\n')
        
        print('Send Bob the public key conjugated by my private key: ')
        a_to_b = conjugate(b(a_priv), b(public))
        print(b(a_to_b))
        print('\nDid Bob send anything to me?')

        b_to_a = sys.stdin.readline()
        b_to_a = b(str_to_list(b_to_a[0:-1]))

        print('\nGenerating shared private key... ')
        private_shared = conjugate(b(a_priv), b_to_a)
        print('Done.\nOK, I got our shared private key! It is ')
        print(private_shared)


    elif sys.argv[1] == 'bob':
        
        print('\nI\'m Bob and I will randomly generate my private key using ' + str(b.gens()[:(n/2-1)]))
        print('\nGenerating private key... ')
        
        # Private key for Bob in the lower subgroup (<a1 ... a_n/2-1>)
        list_to_choose = range(-(n/2-1), 0) + range(1, n/2)
        #b_priv = [list_to_choose[randint(0, len(list_to_choose)-1)] for i in range(7)]
        b_priv = [1,4,-2,-2,3,3,3]
        print('Done\n')
        
        print('Tell me which is Alice\'s public key: ')
        public = str_to_list(sys.stdin.readline())
        
        print('\nNow, send Alice the public key conjugated by my private key: ')
        b_to_a = conjugate(b(b_priv), b(public))
        print(b(b_to_a))
        
        print('\nDid Alice send anything to me?')        
        a_to_b = sys.stdin.readline()
        a_to_b = b(str_to_list(a_to_b[0:-1]))

        print('\nGenerating shared private key... ')
        private_shared = conjugate(b(b_priv), a_to_b)
        print('Done.\nOK, I got our shared private key! It is ')
        print(private_shared)

        
    print('\nIt may not look like my friend\'s key, but it is! This is its \'normal form\'\n')
    normal = private_shared.left_normal_form()
    # We print the power of the fundamental braid
    index = str(normal[0]).find(')^')
    print str(normal[0])[index+2:]
    for elem in normal[1:]:
        print str_to_list(str(elem)) 

if __name__=='__main__':
    main()

