def func(str):
    characterCount=0
    numberCount=0
    for i in str:
        if '0'<=i<='9':
            numberCount+=1
        elif 'a'<=i<='z' or 'Z'<=i<='Z':
            characterCount+=1
    
    return characterCount,numberCount
    
