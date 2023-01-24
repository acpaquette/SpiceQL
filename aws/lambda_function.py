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
        ret = func(**event)
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

