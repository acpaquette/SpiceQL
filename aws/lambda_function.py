import json
import redis
import pyspiceql
import os

def lambda_handler(event, context):
    redis_conn = None
    redis_endpoint = None
    redis_port = None

    if "REDIS_HOST" in os.environ and "REDIS_PORT" in os.environ:
        redis_endpoint = os.environ["REDIS_HOST"]
        redis_port = os.environ["REDIS_PORT"]
        redis_conn = redis.Redis(host=redis_endpoint, port=redis_port)
        redis_conn.set("foo", "bar")
        print(redis_conn.get("foo"))
        del redis_conn

    try: 
        func = getattr(pyspiceql, event["func"])
        event.pop('func')
        args = list(event.values())
        # removes empty arguments from list
        args = [arg for arg in args if arg]
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

