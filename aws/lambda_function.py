import json

def lambda_handler(event, context):
    import sys

    print(sys.executable)
    
    # TODO implement
    print(f"event: ", event)
    print(f"context: ", context)
    
    import pyspiceql
    print(dir(pyspiceql)) 
    
    try: 
        func = getattr(pyspiceql, event["func"])
        event.pop('func')
        args = list(event.values())
        # removes empty arguments from list
        args = [arg for arg in args if arg is not None]
        print(args)
        ret = func(*args)
        print(ret) 
        return {
            "statusCode" : 200,
            "body" : json.dumps({
                "return" : ret
            })
        }
        
    except Exception as e: 
        return {
            "statusCode" : 500,
            "body" : json.dumps({
                "error" : str(e)
            })
        }

