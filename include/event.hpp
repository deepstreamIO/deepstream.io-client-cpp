/*
 * Copyright 2017 deepstreamHub GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DEEPSTREAM_EVENT_HPP
#define DEEPSTREAM_EVENT_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace deepstream
{
	struct Buffer;
	struct Message;


	struct Event
	{
		typedef std::string Name;

		typedef std::function<void(const Buffer&)> SubscribeFn;
		typedef std::shared_ptr<SubscribeFn> SubscribeFnRef;
		typedef std::vector<SubscribeFnRef> SubscriberList;
		typedef std::map<Name, SubscriberList> SubscriberMap;

		typedef std::function<void(const Name&, const Buffer&)> ListenFn;
		typedef std::shared_ptr<ListenFn> ListenFnRef;
		typedef std::map<Name, ListenFnRef> ListenerMap;

		/**
		 * Function matching this signature are expected to return `true` if the
		 * message was sent successfully, `false` otherwise.
		 */
		typedef std::function<bool(const Message&)> SendFn;


		explicit Event(const SendFn&);


		void subscribe(const Name&, const SubscribeFnRef&);
		void unsubscribe(const Name&);
		void unsubscribe(const Name&, const SubscribeFnRef&);

		void listen(const std::string& pattern, const ListenFnRef&);
		void unlisten(const std::string& pattern);


		void notify_(const Message&);
		void notify_subscribers_(const Name&, const Buffer&);


		SendFn send_;
		SubscriberMap subscriber_map_;
		ListenerMap listener_map_;
	};
}

#endif
