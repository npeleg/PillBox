import boto3
from cred import *
from utils import *


PHONE_NUMBER = '+972525977128'
MESSAGE = 'Test From Python'

sns = boto3.client('sns', aws_access_key_id = AWS_ID, aws_secret_access_key = AWS_SECRET_KEY, region_name='us-west-2')

#sns.set_sms_attributes(attributes={"DefaultSenderID": SENDER_ID})
#subs = getTopicSubscribers(sns, 'new-topic')
#sns.publish(PhoneNumber=PHONE_NUMBER, Message=MESSAGE)
#topic = getAllTopics(sns)


#subs = ['0525977128', '0503728949', '0524235432']
#setTopicSubscribers(sns, 'new-topic', subs)

#print(subs)
#print(topic)
